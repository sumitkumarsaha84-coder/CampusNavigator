#include "CampusGraph.hpp"
#include "CampusData.hpp"
#include "TerminalUI.hpp"
#include "HttpServer.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Enable ANSI escape sequence processing on Windows console
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        campus::CampusGraph campusMap;
        campus::CampusData::loadDefaultCampus(campusMap);

        bool cliMode = false;
        int port = 8080;
        std::string webRoot = "web";

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--cli" || arg == "-c") {
                cliMode = true;
            } else if (arg == "--web" || arg == "-w") {
                cliMode = false;
            } else if (arg == "--port" || arg == "-p") {
                if (i + 1 < argc) {
                    port = std::stoi(argv[++i]);
                }
            } else if (arg == "--webroot") {
                if (i + 1 < argc) {
                    webRoot = argv[++i];
                }
            }
        }

        if (cliMode) {
            campus::TerminalUI ui(campusMap);
            ui.run();
        } else {
            campus::HttpServer server(campusMap, port, webRoot);
            server.start(true); // Synchronous server loop
        }
    } catch (const std::exception& ex) {
        std::cerr << "\n[Fatal Error]: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n[Fatal Error]: Unknown runtime exception encountered." << std::endl;
        return 1;
    }

    return 0;
}
