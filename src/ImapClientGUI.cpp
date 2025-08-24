// ImapClientGUI.cpp
// author: Marek Tenora
// login: xtenor02

#include "ImapClientGUI.h"
#include <iostream>
#include <fstream>

ImapClientGUI::ImapClientGUI()
    : m_VBox_Main(Gtk::ORIENTATION_VERTICAL, 5),
      m_Frame_Server("Server Settings"),
      m_Frame_Auth("Authentication"),
      m_Frame_Options("Options"),
      m_Frame_Output("Output"),
      m_Grid_Server(),
      m_Grid_Auth(),
      m_Grid_Options(),
      m_Grid_Output(),
      m_Label_Server("Server:"),
      m_Label_Port("Port:"),
      m_Entry_Server(),
      m_Entry_Port(),
      m_Check_TLS("Use TLS"),
      m_Label_Username("Username:"),
      m_Label_Password("Password:"),
      m_Entry_Username(),
      m_Entry_Password(),
      m_Label_Mailbox("Mailbox:"),
      m_Entry_Mailbox(),
      m_Check_NewMessages("Download only new messages"),
      m_Check_HeadersOnly("Download headers only"),
      m_Label_OutputDir("Output Directory:"),
      m_Entry_OutputDir(),
      m_Button_Browse("Browse..."),
      m_HBox_Buttons(Gtk::ORIENTATION_HORIZONTAL, 5),
      m_Button_Connect("Connect and Download"),
      m_Frame_Status("Status"),
      m_VBox_Status(Gtk::ORIENTATION_VERTICAL, 5),
      m_Label_Status("Ready"),
      m_ProgressBar(),
      m_ScrolledWindow(),
      m_TextView_Log(),
      m_WorkerThread(nullptr),
      m_WorkerActive(false) {

    set_title("IMAP Client GUI");
    set_default_size(600, 700);
    set_border_width(10);

    // Setup main layout
    add(m_VBox_Main);

    // Setup server settings frame
    m_Grid_Server.set_column_spacing(10);
    m_Grid_Server.set_row_spacing(5);
    m_Grid_Server.set_border_width(10);
    
    m_Grid_Server.attach(m_Label_Server, 0, 0, 1, 1);
    m_Grid_Server.attach(m_Entry_Server, 1, 0, 2, 1);
    m_Grid_Server.attach(m_Label_Port, 0, 1, 1, 1);
    m_Grid_Server.attach(m_Entry_Port, 1, 1, 1, 1);
    m_Grid_Server.attach(m_Check_TLS, 2, 1, 1, 1);
    
    m_Entry_Server.set_placeholder_text("imap.example.com");
    m_Entry_Port.set_text("993");
    m_Check_TLS.set_active(true);
    
    m_Frame_Server.add(m_Grid_Server);
    m_VBox_Main.pack_start(m_Frame_Server, Gtk::PACK_SHRINK);

    // Setup authentication frame
    m_Grid_Auth.set_column_spacing(10);
    m_Grid_Auth.set_row_spacing(5);
    m_Grid_Auth.set_border_width(10);
    
    m_Grid_Auth.attach(m_Label_Username, 0, 0, 1, 1);
    m_Grid_Auth.attach(m_Entry_Username, 1, 0, 1, 1);
    m_Grid_Auth.attach(m_Label_Password, 0, 1, 1, 1);
    m_Grid_Auth.attach(m_Entry_Password, 1, 1, 1, 1);
    
    m_Entry_Password.set_visibility(false);
    
    m_Frame_Auth.add(m_Grid_Auth);
    m_VBox_Main.pack_start(m_Frame_Auth, Gtk::PACK_SHRINK);

    // Setup options frame
    m_Grid_Options.set_column_spacing(10);
    m_Grid_Options.set_row_spacing(5);
    m_Grid_Options.set_border_width(10);
    
    m_Grid_Options.attach(m_Label_Mailbox, 0, 0, 1, 1);
    m_Grid_Options.attach(m_Entry_Mailbox, 1, 0, 1, 1);
    m_Grid_Options.attach(m_Check_NewMessages, 0, 1, 2, 1);
    m_Grid_Options.attach(m_Check_HeadersOnly, 0, 2, 2, 1);
    
    m_Entry_Mailbox.set_text("INBOX");
    
    m_Frame_Options.add(m_Grid_Options);
    m_VBox_Main.pack_start(m_Frame_Options, Gtk::PACK_SHRINK);

    // Setup output frame
    m_Grid_Output.set_column_spacing(10);
    m_Grid_Output.set_row_spacing(5);
    m_Grid_Output.set_border_width(10);
    
    m_Grid_Output.attach(m_Label_OutputDir, 0, 0, 1, 1);
    m_Grid_Output.attach(m_Entry_OutputDir, 1, 0, 1, 1);
    m_Grid_Output.attach(m_Button_Browse, 2, 0, 1, 1);
    
    m_Entry_OutputDir.set_text("./mails");
    
    m_Frame_Output.add(m_Grid_Output);
    m_VBox_Main.pack_start(m_Frame_Output, Gtk::PACK_SHRINK);

    // Setup control buttons
    m_HBox_Buttons.set_border_width(10);
    m_HBox_Buttons.pack_end(m_Button_Connect, Gtk::PACK_SHRINK);
    m_VBox_Main.pack_start(m_HBox_Buttons, Gtk::PACK_SHRINK);

    // Setup status frame
    m_VBox_Status.set_border_width(10);
    m_VBox_Status.pack_start(m_Label_Status, Gtk::PACK_SHRINK);
    m_VBox_Status.pack_start(m_ProgressBar, Gtk::PACK_SHRINK);
    
    // Setup log text view
    m_RefTextBuffer = Gtk::TextBuffer::create();
    m_TextView_Log.set_buffer(m_RefTextBuffer);
    m_TextView_Log.set_editable(false);
    m_ScrolledWindow.add(m_TextView_Log);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_ScrolledWindow.set_size_request(-1, 200);
    
    m_VBox_Status.pack_start(m_ScrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    m_Frame_Status.add(m_VBox_Status);
    m_VBox_Main.pack_start(m_Frame_Status, Gtk::PACK_EXPAND_WIDGET);

    // Connect signals
    m_Button_Connect.signal_clicked().connect(sigc::mem_fun(*this, &ImapClientGUI::on_connect_clicked));
    m_Button_Browse.signal_clicked().connect(sigc::mem_fun(*this, &ImapClientGUI::on_browse_clicked));
    m_Check_TLS.signal_toggled().connect(sigc::mem_fun(*this, &ImapClientGUI::on_tls_toggled));
    m_Check_NewMessages.signal_toggled().connect(sigc::mem_fun(*this, &ImapClientGUI::on_new_messages_toggled));
    m_Check_HeadersOnly.signal_toggled().connect(sigc::mem_fun(*this, &ImapClientGUI::on_headers_only_toggled));

    // Connect dispatchers for thread-safe GUI updates
    m_Dispatcher_progress.connect(sigc::mem_fun(*this, &ImapClientGUI::update_progress));
    m_Dispatcher_status.connect(sigc::mem_fun(*this, &ImapClientGUI::update_status));
    m_Dispatcher_completed.connect(sigc::mem_fun(*this, &ImapClientGUI::operation_completed));

    show_all_children();
}

ImapClientGUI::~ImapClientGUI() {
    if (m_WorkerThread) {
        m_WorkerActive = false;
        if (m_WorkerThread->joinable()) {
            m_WorkerThread->join();
        }
        delete m_WorkerThread;
    }
}

void ImapClientGUI::on_connect_clicked() {
    if (m_WorkerActive) {
        append_log("Operation already in progress...", true);
        return;
    }

    // Validate inputs
    if (m_Entry_Server.get_text().empty()) {
        append_log("Error: Server field is required", true);
        return;
    }
    if (m_Entry_Username.get_text().empty()) {
        append_log("Error: Username field is required", true);
        return;
    }
    if (m_Entry_Password.get_text().empty()) {
        append_log("Error: Password field is required", true);
        return;
    }
    if (m_Entry_OutputDir.get_text().empty()) {
        append_log("Error: Output directory is required", true);
        return;
    }

    // Disable connect button and start operation
    m_Button_Connect.set_sensitive(false);
    m_ProgressBar.set_fraction(0.0);
    append_log("Starting IMAP connection...");

    // Start worker thread
    m_WorkerActive = true;
    m_WorkerThread = new std::thread(&ImapClientGUI::worker_thread, this);
}

void ImapClientGUI::on_browse_clicked() {
    Gtk::FileChooserDialog dialog("Choose Output Directory",
                                  Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.set_transient_for(*this);

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Select", Gtk::RESPONSE_OK);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        m_Entry_OutputDir.set_text(dialog.get_filename());
    }
}

void ImapClientGUI::on_tls_toggled() {
    if (m_Check_TLS.get_active()) {
        m_Entry_Port.set_text("993");
    } else {
        m_Entry_Port.set_text("143");
    }
}

void ImapClientGUI::on_new_messages_toggled() {
    // Implementation can be added if needed
}

void ImapClientGUI::on_headers_only_toggled() {
    // Implementation can be added if needed
}

void ImapClientGUI::update_progress() {
    m_Label_Status.set_text(m_ProgressMessage);
    m_ProgressBar.pulse();
}

void ImapClientGUI::update_status() {
    append_log(m_StatusMessage, m_StatusIsError);
}

void ImapClientGUI::operation_completed() {
    m_Button_Connect.set_sensitive(true);
    m_ProgressBar.set_fraction(1.0);
    m_Label_Status.set_text("Operation completed");
    m_WorkerActive = false;
}

void ImapClientGUI::worker_thread() {
    try {
        // Build program options from GUI inputs
        ProgramOptions options = build_program_options();
        AuthData authData = build_auth_data();

        // Update status
        m_StatusMessage = "Connecting to IMAP server...";
        m_StatusIsError = false;
        m_Dispatcher_status.emit();

        // Create and run IMAP client
        ImapClient imapClient(options);
        int result = imapClient.run(authData);

        if (result == 0) {
            m_StatusMessage = "Successfully downloaded emails!";
            m_StatusIsError = false;
        } else {
            m_StatusMessage = "IMAP operation failed";
            m_StatusIsError = true;
        }
        m_Dispatcher_status.emit();

    } catch (const std::exception& e) {
        m_StatusMessage = std::string("Error: ") + e.what();
        m_StatusIsError = true;
        m_Dispatcher_status.emit();
    }

    m_Dispatcher_completed.emit();
}

ProgramOptions ImapClientGUI::build_program_options() {
    ProgramOptions options;
    
    options.server = m_Entry_Server.get_text();
    options.port = std::stoi(m_Entry_Port.get_text());
    options.useTLS = m_Check_TLS.get_active();
    options.onlyNewMessages = m_Check_NewMessages.get_active();
    options.headersOnly = m_Check_HeadersOnly.get_active();
    options.mailbox = m_Entry_Mailbox.get_text();
    options.outputDir = m_Entry_OutputDir.get_text();
    
    // For GUI, we don't use auth file
    options.authFile = "";
    
    return options;
}

AuthData ImapClientGUI::build_auth_data() {
    AuthData authData;
    authData.username = m_Entry_Username.get_text();
    authData.password = m_Entry_Password.get_text();
    return authData;
}

void ImapClientGUI::append_log(const std::string& message, bool is_error) {
    auto iter = m_RefTextBuffer->end();
    
    if (is_error) {
        m_RefTextBuffer->insert_with_tag(iter, message + "\n", m_RefTextBuffer->create_tag());
        // TODO: Add red color tag for errors
    } else {
        m_RefTextBuffer->insert(iter, message + "\n");
    }
    
    // Auto-scroll to bottom
    auto mark = m_RefTextBuffer->get_insert();
    m_TextView_Log.scroll_to(mark);
}