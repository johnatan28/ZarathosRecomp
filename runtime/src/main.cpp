#include "ps2_runtime.h"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace
{
    void setupTerminateLogger()
    {
        std::set_terminate([]()
                           {
                               std::cerr << "[ghostcomp][terminate] unhandled exception" << std::endl;
                               const std::exception_ptr ep = std::current_exception();
                               if (ep)
                               {
                                   try
                                   {
                                       std::rethrow_exception(ep);
                                   }
                                   catch (const std::system_error &e)
                                   {
                                       std::cerr << "[ghostcomp][terminate] std::system_error code="
                                                 << e.code().value()
                                                 << " category=" << e.code().category().name()
                                                 << " message=" << e.what() << std::endl;
                                   }
                                   catch (const std::exception &e)
                                   {
                                       std::cerr << "[ghostcomp][terminate] std::exception: "
                                                 << e.what() << std::endl;
                                   }
                                   catch (...)
                                   {
                                       std::cerr << "[ghostcomp][terminate] non-std exception" << std::endl;
                                   }
                               }
                               std::abort();
                           });
    }

    std::filesystem::path resolveElfPath(int argc, char *argv[])
    {
        if (argc >= 2 && argv[1] && argv[1][0] != '\0')
        {
            return std::filesystem::path(argv[1]);
        }
#if defined(GHOSTCOMP_DEFAULT_ELF)
        const std::filesystem::path defaultPath(GHOSTCOMP_DEFAULT_ELF);
        if (defaultPath.is_absolute())
        {
            return defaultPath;
        }
        return (std::filesystem::current_path() / defaultPath).lexically_normal();
#else
        throw std::runtime_error("Pass the guest PS2 ELF path as argv[1] (e.g. game/SLUS_213.06).");
#endif
    }
}

int main(int argc, char *argv[])
{
    setupTerminateLogger();

    try
    {
        const std::filesystem::path elfPath = resolveElfPath(argc, argv);
        const std::string elfPathStr = elfPath.string();
        const std::string elfName = elfPath.filename().string();

        std::cout << "[ZarathosRecomp] Starting Ghost Rider (PS2) native PC recompilation runner" << std::endl;
        std::cout << "[ZarathosRecomp] Guest ELF: " << elfPathStr << std::endl;

        if (!std::filesystem::exists(elfPath))
        {
            std::cerr << "[ZarathosRecomp] Error: ELF not found at '" << elfPathStr << "'." << std::endl;
            std::cerr << "[ZarathosRecomp] Place your legally extracted SLUS_213.06 in game/ or pass its path as argv[1]." << std::endl;
            return 1;
        }

        const std::string windowTitle = "ZarathosRecomp | Ghost Rider (PS2) | " + elfName;

        PS2Runtime runtime;
        if (!runtime.initialize(windowTitle.c_str()))
        {
            std::cerr << "[ZarathosRecomp] Failed to initialize PS2Runtime." << std::endl;
            return 1;
        }

        if (!runtime.loadELF(elfPathStr))
        {
            std::cerr << "[ZarathosRecomp] Failed to load ELF: " << elfPathStr << std::endl;
            return 1;
        }

        std::cout << "[ZarathosRecomp] ELF loaded successfully. Entering recompiled guest execution..." << std::endl;
        runtime.run();

        std::cout.flush();
        std::cerr.flush();
        std::_Exit(0);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ghostcomp] Fatal exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[ghostcomp] Fatal unknown exception." << std::endl;
    }

    std::cout.flush();
    std::cerr.flush();
    std::_Exit(1);
}
