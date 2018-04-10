// std
#include <string>
#include <vector>
// 3rd party libs.
#include "json.h"
using json = nlohmann::json;
// AmazingCow Libs
#include "acow/algo.h"
#include "acow/ConioEx.h"
#include "acow/URLDownloadFile.h"
#include "CMD/CMD.h"
#include "CoreDir/CoreDir.h"
#include "CoreFile/CoreFile.h"
#include "CoreFS/CoreFS.h"
#include "CoreLog/CoreLog.h"
#include "CoreString/CoreString.h"


//----------------------------------------------------------------------------//
// Types                                                                      //
//----------------------------------------------------------------------------//
struct repo_t {
	std::string clone_url;
	std::string name;
	std::string organization;
};


//----------------------------------------------------------------------------//
// Helper Functions                                                           //
//----------------------------------------------------------------------------//
acow_internal_function CoreLog::Logger*
GetLogger() noexcept 
{ 
    acow_local_persist CoreLog::Logger s_logger;
    return &s_logger;
}

acow_internal_function void
ShowHelp(const std::string &helpString) noexcept 
{
    printf("%s\n", helpString.c_str());
    exit(0);
}

acow_internal_function void
ShowVersion() noexcept 
{
    // COWTODO(n2omatt): Implement version...
    printf("Version: ");
    exit(0);
}

acow_internal_function void 
PrintLineSeparator(const std::string &caption) noexcept
{
    auto console_width = acow::conio::ex::GetTerminalWidth();
    auto line_width    = (console_width - caption.size() - 2) / 2;
    auto line_str      = std::string(line_width, '-');
    
    printf(
        "%s %s %s\n", 
        line_str.c_str(), 
        caption.c_str(), 
        line_str.c_str()
    );
}

//----------------------------------------------------------------------------//
// Functions                                                                  //
//----------------------------------------------------------------------------//
acow_internal_function std::vector<repo_t>
FindReposForOrganization(const std::string &organization) noexcept
{
	constexpr auto kOrganizationUrl_Fmt			= "https://api.github.com/users/%s/repos";
	constexpr auto kOrganizationUrl_Buffer_Size = 1024;

	// Build the Organiation Url.
	char organization_url_buff[kOrganizationUrl_Buffer_Size] = {};
	sprintf(organization_url_buff, kOrganizationUrl_Fmt, organization.c_str());

	// Download from GitHub the Organization data.
	std::string json_str;
	auto success = acow::URLDownloadFile::DownloadString(
		organization_url_buff,
		&json_str
	);

	if (!success) {
		// COWTODO(n2omatt): Handle error...
	}

	// Parse the downloaded data and make the repos list.
	auto json = json::parse(json_str.c_str());

	std::vector<repo_t> repos;
	for (auto &info : json) {
		repo_t repo;

		repo.clone_url    = info["clone_url"].get<std::string>();;
		repo.name	      = info["name"]     .get<std::string>();;
		repo.organization = organization;

		repos.push_back(repo);
	}

	return repos;
}

acow_internal_function void
CloneRepository(const repo_t &repo, const std::string &fullpath) noexcept
{
    auto repo_fullpath = CoreFS::Join(fullpath, {repo.name});
    if(CoreFS::IsDir(repo_fullpath)) {
        GetLogger()->Info(
            "Repository already cloned - Repo: (%s) - Path: (%s)",
            repo.name, 
            repo_fullpath
        );
        
        return;
    }

    // Change the CWD.
    //   Needed to ease the git operations.
    auto old_cwd = CoreDir::GetCurrentDirectory();
    CoreDir::SetCurrentDirectory(fullpath);

    GetLogger()->I("Cloning repo: %s\n", repo.name.c_str());
    auto clone_cmd = CoreString::Format(
        "git clone %s",
        repo.clone_url
    );

    system(clone_cmd.c_str());

    // Restore the CWD.
    CoreDir::SetCurrentDirectory(old_cwd);
}


//----------------------------------------------------------------------------//
// Entry Point                                                                //
//----------------------------------------------------------------------------//
int 
main(int argc, char *argv[])
{	
    // Help / Version
    auto flag_help = CMD::Flag::Create(
        "h", "help", "Show this screen.", 
        CMD::NoArgument | CMD::StopOnView
    );
    auto flag_version = CMD::Flag::Create(
        "v", "version", "Show the version information.",
        CMD::NoArgument | CMD::StopOnView
    );

    // Control Flags.
    auto flag_verbose = CMD::Flag::Create(
        "V", "verbose", "Show extra output.", 
        CMD::NoArgument | CMD::AllowDuplicates
    );

    // Info Flags.
    auto flag_output = CMD::Flag::Create(
        "o", "output-path", "Base directory where the repos will be cloned.", 
        CMD::RequiredArgument | CMD::NoDuplicates
    );
    auto flag_organizations = CMD::Flag::Create(
        "O", "organization", "Organization name.", 
        CMD::RequiredArgument | CMD::AllowDuplicates | CMD::MergeArgument
    );
    auto flag_organizations_file = CMD::Flag::Create(
        "f", "orgs-file", "Set the input organizations file.",
        CMD::RequiredArgument | CMD::NoDuplicates
    );

    auto parser = CMD::Parser(
        CMD::ParserOptions::DieOnNonValidFlags, 
        {
            flag_help, 
            flag_version, 
            flag_verbose, 
            flag_output, 
            flag_organizations,
            flag_organizations_file
        }
    );
    parser.Parse(argc, argv);
    
    // Help / Version.
    if(flag_help   ->Found()) { ShowHelp(parser.GenerateHelpString()); }
    if(flag_version->Found()) { ShowVersion();                         }        

    // Control.
    if(flag_verbose->Found()) { 
        GetLogger()->SetLevel(CoreLog::Logger::LOG_LEVEL_VERBOSE);
    } else {
        GetLogger()->SetLevel(CoreLog::Logger::LOG_LEVEL_NONE);
    }

    // Info flags.
    if(!flag_output->Found()) {
        GetLogger()->Fatal("Missing output path.");
    }

    if(!flag_organizations->Found() && !flag_organizations_file) { 
        GetLogger()->Fatal("Missing organizations names.");
    }

    // Get the Output path.
    auto output_path = flag_output->GetValue ();
    
    // Get the organizations.
    std::vector<std::string> organizations; 
    if(flag_organizations->Found()) { // From flags.
        acow::algo::copy_insert(flag_organizations->GetValues(), organizations);
    }
    if(flag_organizations_file->Found()) { // From file.
        auto org_filename = CoreFS::ExpandUserAndMakeAbs(
            flag_organizations_file->GetValue()
        );

        if(!CoreFS::IsFile(org_filename)) { 
            GetLogger()->Fatal(
                "Invalid Organizations File - Filename: (%s)", 
                org_filename
           );
        }
        auto file_lines = CoreFile::ReadAllLines(org_filename);
        acow::algo::copy_insert(file_lines, organizations);
    }
    acow::algo::sort_and_unique(organizations, true);

 
    // Create the output directory.
	auto output_fullpath = CoreFS::ExpandUserAndMakeAbs(output_path);
	CoreDir::CreateDirectory(output_fullpath);

    for(auto &organization : organizations) { 
        organization = CoreString::Trim(organization);
        PrintLineSeparator(organization);

        auto repos = FindReposForOrganization(organization);
        if(repos.empty()) { 
            GetLogger()->Info(
                "Organization is empty - Organization: (%s)", 
                organization
            );
            
            continue;
        }

        auto organization_fullpath = CoreFS::Join(output_fullpath, {organization});
        CoreDir::CreateDirectory(organization_fullpath);

        for(const auto &repo : repos) { 
            CloneRepository(repo, organization_fullpath);
        }        
    }
}
