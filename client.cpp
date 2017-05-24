#include "client.h"
#include <boost/property_tree/xml_parser.hpp>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/panel.h>
#include <wx/ribbon/page.h>
#include <config.h>
#include <iostream>
#include <listener.h>
#include <memory>
#include <stdint.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/xrc/xh_ribbon.h>
#include <wx/button.h>
#include "text_panel.h"
#include "player_list.h"
using namespace std::string_literals;
class DerivedApp : public wxApp
{
    lsy::listener                li;
    wxDialog*                    dlg;
    wxFrame*                     mf;
    lsy::as_ptr< lsy::port_all > pa;
	std::map<std::string, text_panel*> channel_map;
	pl_panel *player_pannel;

  public:
    virtual bool OnInit();
    virtual int  OnExit();
    void init_dating();
	void init_play();
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
	pa->close();
    li.join();
    return 0;
}

void set_text(wxStaticText *st, std::tuple<uint16_t, uint16_t>*p)
{
	if (std::get<1>(*p))
	{
			wxString str = wxString::Format(wxT("开始匹配成功:%u/%u"), std::get<0>(*p), std::get<1>(*p));
			if (std::get<1>(*p) == std::get<0>(*p)) {
				str += "\n 请使用游戏标签页进行游戏";
			}
			st->SetLabel(str);
	}
	else
	{
		st->SetLabel(wxT("开始匹配失败，请刷新房间列表后重试"));
	}
}

void DerivedApp::init_dating()
{

		mf = wxXmlResource::Get()->LoadFrame(NULL, "MyFrame2");
		mf->Show();
		auto boptr = wxStaticCast(
			wxWindow::FindWindowByName("m_notebook1", mf), wxNotebook);

		//init mutiplayer
		auto mutipanel
			= wxXmlResource::Get()->LoadPanel(boptr, "mutipanel");
		boptr->AddPage(mutipanel, "多人游戏");
		auto pp = new std::tuple<uint16_t, uint16_t>();
		auto mp=pa->resign_port(config::match_port);
		mp->OnMessage.connect([pp,this](lsy::buffer buf) {
			buf.get(std::get<1>(*pp));
			gui_run([pp,this]() {
				set_text(XRCCTRL(*mf,"m_staticText3", wxStaticText), pp);
				});
		});
		mp->start();
		auto msp = pa->resign_port(config::match_status_port);
		msp->OnMessage.connect([pp,this](lsy::buffer buf) {
			buf.get(std::get<0>(*pp));
			gui_run([pp, this]() {
				set_text(XRCCTRL(*mf, "m_staticText3", wxStaticText), pp);
				if (std::get<0>(*pp) == std::get<1>(*pp))
				{
					init_play();
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
                    wxWindow::FindWindowByName("m_staticText5", mf),
                    wxStaticText)
                    ->SetLabel("加载完毕");
                mf->Refresh();
            }
            else
            {
                wxStaticCast(wxWindow::FindWindowByName("m_listBox1", mf),
                                wxListBox)
                    ->Append(wxString((char*)buf.data()));
                mf->Refresh();
            }
        });
		po->start();
		std::function<void(wxCommandEvent &)> func= [this](auto a) {
			pa->ports[config::multiplay_port]->write(lsy::buffer((size_t)0), []() {});
			XRCCTRL(*mf, "m_listBox1", wxListBox)->Clear();
			XRCCTRL(*mf, "m_staticText5", wxStaticText)->SetLabel("加载中");
			mf->Refresh();
		};
		XRCCTRL(*mf, "m_button5", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, func);
		func(wxCommandEvent());
		XRCCTRL(*mf,"m_button4", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &event) {
			wxListBox *lb = XRCCTRL(*mf, "m_listBox1", wxListBox);
			wxString str=lb->GetString(lb->GetSelection());
			pa->ports[config::match_port]->write(str.ToStdString(), []() {});
		});
		pa->ports[config::match_port]->write("sryx"s, []() {});
}
void DerivedApp::init_play()
{
	auto boptr = XRCCTRL(*mf, "m_notebook1", wxNotebook);
	auto playerpanel
		= wxXmlResource::Get()->LoadPanel(boptr, "playpanel");
	boptr->AddPage(playerpanel, "游戏", true);
	auto ch_con = pa->resign_port(config::channel_open);
	ch_con->start();
	auto ch_en = pa->resign_port(config::channel_enable);
	auto ro_info = pa->resign_port(config::room_info);
	ro_info->start();
	ro_info->write(lsy::buffer(size_t(0)), []() {});
	ro_info->OnMessage.connect([this](auto buf) {
		std::string str((char*)buf.get(0));
		buf.get(str.size() + 1);
		uint8_t num;
		buf.get(&num, 1);
		gui_run([num,str, playerpanel =XRCCTRL(*mf,"playpanel",wxPanel)]() {
			auto left = XRCCTRL(*mf, "m_ribbonPage112", wxRibbonPage);
			auto right = XRCCTRL(*mf, "m_ribbonPage114", wxRibbonPage);
			player_pannel =new pl_panel(left, right, num);
			left->Realize();
			right->Realize();
			XRCCTRL(*mf, "m_staticText28", wxStaticText)->SetLabel(wxString::Format("你的身份是：%s\n",str));
			playerpanel->Refresh();
		});
		auto bupo = pa->resign_port(config::button_port);
		bupo->start();
		bupo->OnMessage.connect([this](auto buf) {
			uint16_t port;
			buf.get(port);
			std::string name((char*)buf.get(0));
			auto po = pa->resign_port(port);
			po->start();
			po->OnMessage.connect([name, po, this](auto buf) {
				gui_run([name, po, this]() {
					for (auto a : player_pannel->pannels)
					{
						a->remove(name);
					}
					XRCCTRL(*mf, "m_ribbonPage112", wxRibbonPage)->Realize();
					XRCCTRL(*mf, "m_ribbonPage114", wxRibbonPage)->Realize();
					mf->Refresh();
					po->close();
				});
			});
			gui_run([name, po, this]() {
				uint8_t index = 0;
				for (auto a : player_pannel->pannels)
				{
					a->add(name, [po, index]() {
						auto in = index;
						lsy::buffer buf(size_t(1));
						buf.put(&in, 1);
						po->write(buf, []() {});
					});
					index++;
				}
				XRCCTRL(*mf, "m_ribbonPage112", wxRibbonPage)->Realize();
				XRCCTRL(*mf, "m_ribbonPage114", wxRibbonPage)->Realize();
				mf->Refresh();
			});
		});
		bupo->write(lsy::buffer(size_t(0)), []() {});
	});
	auto pl_de = pa->resign_port(config::player_dead);
	pl_de->OnMessage.connect([this](auto buf) {
		uint8_t a;
		uint8_t b;
		buf.get(&a,1);
		buf.get(&b ,1); 
		gui_run([a, b,this]() {		
			player_pannel->set_status(a, b);
			mf->Refresh();
		});
	});
	pl_de->start();
	auto ch_cl = pa->resign_port(config::channel_close);
	//ch_en->start();
	/*
	wxGridSizer* gSizer1=new wxGridSizer(0,2,0,0);
	playerpanel->GetSizer()->Add(gSizer1);
	auto pbm = new wxStaticBitmap(playerpanel, wxID_ANY, wxBitmap("qbt.bmp", wxBITMAP_TYPE_BMP), wxDefaultPosition, wxSize(70, 70));
	gSizer1->Add(pbm,wxALL);
	pbm = new wxStaticBitmap(playerpanel, wxID_ANY, wxBitmap("qbt.bmp", wxBITMAP_TYPE_BMP), wxDefaultPosition, wxSize(70, 70));
	gSizer1->Add(pbm, wxALL);
	playerpanel->GetSizer()->Layout();*/
	//new wxTextCtrl(rp, wxID_ANY, "test");
	//new wxRibbonPanel(rp, wxID_ANY, wxT("asda"));
	//new wxRibbonPanel(rp, wxID_ANY, wxT("asdn"));
	mf->Layout();
	mf->Refresh();
	mf->Show();
	ch_cl->OnMessage.connect([this](auto buf) {
		printf("in:%d\n",buf.size());
		std::string name((char*)buf.get(0));
		auto it=channel_map.find(name);
		if (it == channel_map.end())
		{
			printf("removing no-exist channel%s\n", name.c_str());
		}
		else
		{
			wxRibbonBar *bar = XRCCTRL(*mf, "m_ribbonBar5", wxRibbonBar);
			if (bar->GetPageCount() == 1) {
				printf("cannot delete the only channel\n");
			}
			else {
				int m;
				int n = bar->GetPageNumber(wxStaticCast(it->second->GetParent(), wxRibbonPage));
				if (n == bar->GetActivePage()) {
					if (n == 0) {
						m = n + 1;
					}
					else
					{
						m = n - 1;
					}
					bar->SetActivePage(m);
				}
				bar->DeletePage(n);
				bar->Realize();
				bar->Refresh();
			}
		}
	});
	ch_cl->start();
	ch_en->OnMessage.connect([this](auto buf) {
		uint16_t b;
		buf.get(b);
		std::string name((char*)buf.get(1));
		if (channel_map.find(name) != channel_map.end())
		{
			gui_run([this, name, b]() {
				channel_map[name]->Enable(b);
				mf->Refresh();
			});
		}
	});
	ch_en->start();
	ch_con->OnMessage.connect([playerpanel, this](auto buf) {
		uint16_t op;
		buf.get(op);
		std::string name((char *)buf.get(buf.remain()));
		if (channel_map.find(name) == channel_map.end())
		{
			gui_run([name, playerpanel, this, op]() {
				wxRibbonBar *bar = XRCCTRL(*mf, "m_ribbonBar5", wxRibbonBar);
				auto rp = new wxRibbonPage(bar, wxID_ANY, name);
				auto po = pa->resign_port(op);
				auto tp = new text_panel(po,rp);
				channel_map[name] = tp;
				XRCCTRL(*mf, "m_ribbonBar5", wxRibbonBar)->Realize();
				mf->Maximize(false);
				mf->Maximize(true);
			});
		}
		else {
			printf("warning muti create channel\n");
		}
	});
	ch_con->write(lsy::buffer((size_t)0), []() {});
}
#include "player_list.h"
bool DerivedApp::OnInit()
{/*
	wxFrame *f = new wxFrame(0,  - 1, "qwe");
	f->Show(true);
	new pl_panel(f);
	f->Refresh();
	/*/
    wxXmlResource::Get()->InitAllHandlers();
	wxXmlResource::Get()->AddHandler(new wxRibbonXmlHandler);
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
                    gui_run([this]() { init_dating(); });
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
    li.add_group(pt.find("client")->second);/**/
    return true;
}