#include "client.h"
#include <boost/property_tree/xml_parser.hpp>
#include <config.h>
#include <iostream>
#include <listener.h>
#include <memory>
#include <stdint.h>
#include <wx/notebook.h>
class DerivedApp : public wxApp
{
    lsy::listener                li;
    wxDialog*                    dlg;
    wxFrame*                     mf;
    lsy::as_ptr< lsy::port_all > pa;

  public:
    virtual bool OnInit();
    virtual int  OnExit();
};
IMPLEMENT_APP_CONSOLE(DerivedApp);

typedef std::function< void() > func_type;
void gui_run(func_type func)
{
    wxThreadEvent ev;
    ev.SetPayload< func_type >(func);
    wxTheApp->QueueEvent(ev.Clone());
}

int DerivedApp::OnExit()
{
    li.join();
    return 0;
}
bool DerivedApp::OnInit()
{
    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->Load("client.xrc");
    dlg = wxXmlResource::Get()->LoadDialog(NULL, "MyDialog1");
    wxTheApp->Bind(wxEVT_COMMAND_THREAD,
                   [](wxThreadEvent e) { e.GetPayload< func_type >()(); });
    XRCCTRL(*dlg, "canel_b", wxButton)
        ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
               [this](wxCommandEvent a) { dlg->Close(); });
    XRCCTRL(*dlg, "login_b", wxButton)
        ->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent a) {
            if (XRCCTRL(*dlg, "id_tc", wxTextCtrl)->GetLineLength(0) != 0
                && XRCCTRL(*dlg, "passwd_tc", wxTextCtrl)->GetLineLength(0)
                       != 0)
            {
                std::string id(
                    XRCCTRL(*dlg, "id_tc", wxTextCtrl)->GetValue().mb_str());
                std::string passwd(XRCCTRL(*dlg, "passwd_tc", wxTextCtrl)
                                       ->GetValue()
                                       .mb_str());
                lsy::buffer buf(id.size() + passwd.size() + 2);
                char        ch = 0;
                buf.put((unsigned char*)id.data(), id.size() + 1);
                buf.put((unsigned char*)passwd.data(), passwd.size() + 1);
                auto p = pa->ports[config::login_port];
                p->write(buf, []() {
                    std::cout << "login request sent" << std::endl;
                });
                dlg->SetTitle("logining");
                dlg->Refresh();
            }
        });
    dlg->SetTitle("connecting");
    dlg->Show();
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml("client.xml", pt);
    li.OnConnect.connect([this](auto port) {
        pa = port;
        port->OnError.connect(
            [](auto er) { std::cout << er.message() << std::endl; });
        port->start();
        auto p = port->resign_port(config::login_port);
        p->start();
        p->OnMessage.connect([this, p](auto buf) {
            uint16_t flag;
            buf.get(flag);
            switch (flag)
            {
                case 0:
                    gui_run([this]() {
                        mf = wxXmlResource::Get()->LoadFrame(NULL, "MyFrame2");
						//wxMenu *p=XRCCTRL(mf, "m_menu1", wxMenu);
                        mf->Show();
						auto boptr = wxWindow::FindWindowByName("m_notebook1", mf);
						wxStaticCast(boptr, wxNotebook)->AddPage(new wxPanel(boptr), "¶àÈËÓÎÏ·");
						/*
						mf->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent a) {
                                wxWindow::FindWindowByName("muti_panel", mf)
                                    ->Show(true);
								mf->Refresh();
                                auto po
                                    = pa->resign_port(config::multiplay_port);
                                po->OnMessage.connect([this](lsy::buffer buf) {
                                    wxStaticCast(wxWindow::FindWindowByName(
                                                     "m_listBox2", mf),
                                                 wxListBox)
                                        ->Append(wxString((char*)buf.data()));
									mf->Refresh();
                                });
                                po->OnDestroy.connect([this]() {
                                    wxStaticCast(wxWindow::FindWindowByName(
                                                     "m_listBox2", mf),
                                                 wxListBox)
                                        ->Clear();
									wxWindow::FindWindowByName("muti_panel", mf)
										->Show(false);
									mf->Refresh();
                                });
                            }, wxXmlResource::GetXRCID(wxT("m_menubar1")));
							*/
                    });
                    p->close();
                    break;
                case 1:
                    dlg->SetTitle("wrong passwd!");
                    dlg->Refresh();
                    break;
                case 2:
                    dlg->SetTitle("wrong id!");
                    dlg->Refresh();
                    break;
                default:
                    assert(0);
                    break;
            }
        });
        p->OnDestroy.connect([this]() { gui_run([this]() { dlg->Close(); }); });
        gui_run([this]() {
            dlg->SetTitle("connected");
            dlg->Refresh();
            XRCCTRL(*dlg, "login_b", wxButton)->Enable(true);
        });
    });
    li.add_group(pt.find("client")->second);
    return true;
}