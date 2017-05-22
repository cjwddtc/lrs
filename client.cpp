#include "client.h"
#include <boost/property_tree/xml_parser.hpp>
#include <config.h>
#include <iostream>
#include <listener.h>
#include <memory>
#include <stdint.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
using namespace std::string_literals;
class DerivedApp : public wxApp
{
    lsy::listener                li;
    wxDialog*                    dlg;
    wxFrame*                     mf;
    lsy::as_ptr< lsy::port_all > pa;

  public:
    virtual bool OnInit();
    virtual int  OnExit();
    void init_dating(int16_t type);
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

void set_text(wxStaticText *st, std::tuple<uint16_t, uint16_t>*p)
{
	if (std::get<1>(*p))
	{
			wxString str = wxString::Format(wxT("开始匹配成功\n%u/%u"), std::get<0>(*p), std::get<1>(*p));
			if (std::get<1>(*p) == std::get<0>(*p)) {
				str += "\n 请使用游戏标签页进行游戏";
			}
			st->SetLabelText(str);
	}
	else
	{
		st->SetLabelText(wxT("开始匹配失败，请刷新房间列表后重试"));
	}
}

void DerivedApp::init_dating(int16_t type)
{
    switch (type)
    {
        case 0:
        {
            mf = wxXmlResource::Get()->LoadFrame(NULL, "MyFrame2");
            // wxMenu *p=XRCCTRL(mf, "m_menu1", wxMenu);
            mf->Show();
            auto boptr = wxStaticCast(
                wxWindow::FindWindowByName("m_notebook1", mf), wxNotebook);
            auto mutipanel
                = wxXmlResource::Get()->LoadPanel(boptr, "mutipanel");
            boptr->AddPage(mutipanel, "多人游戏");
        }
        case 1:
        {
			auto pp = new std::tuple<uint16_t, uint16_t>();
			auto mp=pa->resign_port(config::match_port);
			mp->start();
			mp->OnMessage.connect([pp,this](lsy::buffer buf) {
				buf.get(std::get<1>(*pp));
				gui_run([pp,this]() {
					set_text(wxStaticCast(
						wxWindow::FindWindowByName("m_staticText3", mf),
						wxStaticText), pp);
				});
			});
			auto msp = pa->resign_port(config::match_status_port);
			msp->OnMessage.connect([pp,this](lsy::buffer buf) {
				buf.get(std::get<0>(*pp));
				gui_run([pp, this]() {
					set_text(wxStaticCast(
						wxWindow::FindWindowByName("m_staticText3", mf),
						wxStaticText), pp);
					if (std::get<0>(*pp) == std::get<1>(*pp))
					{
						auto boptr = wxStaticCast(
							wxWindow::FindWindowByName("m_notebook1", mf), wxNotebook);
						auto playerpanel
							= wxXmlResource::Get()->LoadPanel(boptr, "playpanel");
						boptr->AddPage(playerpanel, "游戏",true);
						auto ch_con = pa->resign_port(config::channel_control);
						ch_con->start();
						ch_con->OnMessage.connect([playerpanel](auto buf) {
							uint16_t op;
							buf.get(op);
							if (op == 0) {
								wxString name(buf.get(buf.remain()));
								gui_run([name, playerpanel]() {
									auto lb = wxStaticCast(wxWindow::FindWindowByName("m_listbook1", playerpanel), wxListbook);
									auto textpanel
										= wxXmlResource::Get()->LoadPanel(lb, "textpanel");
									lb->AddPage(textpanel, name);
									playerpanel->Refresh();
								});
							}
						});
					}
				});
			});
			msp->start();
            auto po = pa->resign_port(config::multiplay_port);
            po->OnMessage.connect([ this, po = lsy::as_ptr< lsy::port >(po) ](
                lsy::buffer buf) {
                if (buf.size() == 1)
                {
                    wxStaticCast(
                        wxWindow::FindWindowByName("m_staticText3", mf),
                        wxStaticText)
                        ->SetLabel("加载完毕");
                    mf->Refresh();
                    po->close();
                }
                else
                {
                    wxStaticCast(wxWindow::FindWindowByName("m_listBox1", mf),
                                 wxListBox)
                        ->Append(wxString((char*)buf.data()));
                    mf->Refresh();
                }
            });
            po->write(lsy::buffer((size_t)0), []() {});
            po->start();
			wxListBox *lb =
				wxStaticCast(wxWindow::FindWindowByName("m_listBox1", mf),
					wxListBox);
            lb->Clear();
            wxStaticCast(wxWindow::FindWindowByName("m_staticText3", mf),
                         wxStaticText)
                ->SetLabel("加载中");
            mf->Refresh();
			wxStaticCast(wxWindow::FindWindowByName("m_button4", mf),
				wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [lb,this](wxCommandEvent &event) {
				wxString str=lb->GetString(lb->GetSelection());
				auto po= pa->ports[config::match_port].get();
				po->write(str.ToStdString(), []() {});
			});
			auto tt = pa->ports[config::match_port].get();
			tt->write("sryx"s, []() {});
        }
        break;
    }
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
                    gui_run([this]() { init_dating(0); });
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