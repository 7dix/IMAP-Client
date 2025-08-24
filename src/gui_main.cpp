// gui_main.cpp
// author: Marek Tenora
// login: xtenor02

#include <gtkmm/application.h>
#include "ImapClientGUI.h"

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.example.imapclient");

    ImapClientGUI window;

    // Start the application main loop
    return app->run(window);
}