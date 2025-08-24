// ImapClientGUI.h
// author: Marek Tenora
// login: xtenor02

#ifndef IMAPCLIENTGUI_H
#define IMAPCLIENTGUI_H

#include <gtkmm.h>
#include <thread>
#include <atomic>
#include "ImapClient.h"
#include "ArgumentsParser.h"
#include "AuthReader.h"

/**
 * @class ImapClientGUI
 * @brief GTK-based GUI wrapper for the IMAP client functionality.
 */
class ImapClientGUI : public Gtk::Window {
public:
    ImapClientGUI();
    virtual ~ImapClientGUI();

protected:
    // Signal handlers:
    void on_connect_clicked();
    void on_browse_clicked();
    void on_tls_toggled();
    void on_new_messages_toggled();
    void on_headers_only_toggled();

    // GUI update methods (called from worker thread)
    void update_progress();
    void update_status();
    void operation_completed();

    // Thread-safe GUI update dispatcher
    Glib::Dispatcher m_Dispatcher_progress;
    Glib::Dispatcher m_Dispatcher_status;
    Glib::Dispatcher m_Dispatcher_completed;

    // Member widgets:
    Gtk::Box m_VBox_Main;
    Gtk::Frame m_Frame_Server, m_Frame_Auth, m_Frame_Options, m_Frame_Output;
    Gtk::Grid m_Grid_Server, m_Grid_Auth, m_Grid_Options, m_Grid_Output;
    
    // Server settings
    Gtk::Label m_Label_Server, m_Label_Port;
    Gtk::Entry m_Entry_Server, m_Entry_Port;
    Gtk::CheckButton m_Check_TLS;
    
    // Authentication
    Gtk::Label m_Label_Username, m_Label_Password;
    Gtk::Entry m_Entry_Username, m_Entry_Password;
    
    // Options
    Gtk::Label m_Label_Mailbox;
    Gtk::Entry m_Entry_Mailbox;
    Gtk::CheckButton m_Check_NewMessages, m_Check_HeadersOnly;
    
    // Output
    Gtk::Label m_Label_OutputDir;
    Gtk::Entry m_Entry_OutputDir;
    Gtk::Button m_Button_Browse;
    
    // Control buttons
    Gtk::Box m_HBox_Buttons;
    Gtk::Button m_Button_Connect;
    
    // Status and progress
    Gtk::Frame m_Frame_Status;
    Gtk::Box m_VBox_Status;
    Gtk::Label m_Label_Status;
    Gtk::ProgressBar m_ProgressBar;
    Gtk::ScrolledWindow m_ScrolledWindow;
    Gtk::TextView m_TextView_Log;
    Glib::RefPtr<Gtk::TextBuffer> m_RefTextBuffer;

private:
    // Worker thread management
    std::thread* m_WorkerThread;
    std::atomic<bool> m_WorkerActive;
    
    // Thread-safe communication
    std::string m_ProgressMessage;
    std::string m_StatusMessage;
    bool m_StatusIsError;
    
    // Worker thread function
    void worker_thread();
    
    // Helper methods
    ProgramOptions build_program_options();
    AuthData build_auth_data();
    void append_log(const std::string& message, bool is_error = false);
};

#endif // IMAPCLIENTGUI_H