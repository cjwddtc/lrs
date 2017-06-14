#include "client.h"
#include <boost/property_tree/xml_parser.hpp>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/panel.h>
#include <wx/ribbon/page.h>
#include <config.h>
#include <boost/lexical_cast.hpp>
#include <wx/filepicker.h>
#include <iostream>
#include <listener.h>
#include <memory>
#include <stdint.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/xrc/xh_ribbon.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/listbox.h>
#include "text_panel.h"
#include "player_list.h"
#include <wx/listctrl.h>
#include <set>
using namespace std::string_literals;
wxFrame*                     mf;
pl_panel *player_pannel;
text_panel *current_channel;
class DerivedApp : public wxApp
{
    lsy::listener                li;
    wxDialog*                    dlg;
	wxDialog*res_dia;
    lsy::as_ptr< lsy::port_all > pa;
	std::map<std::string, text_panel*> channel_map;
	std::set<std::string> buttons;

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
typedef std::tuple<uint16_t, uint16_t, bool> match_data;

void set_text(wxStaticText *st, match_data *p)
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
		st->SetLabel(wxT("开始匹配失败，请不要同时匹配多局游戏"));
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
		auto gamepanel= wxXmlResource::Get()->LoadPanel(boptr, "games_panel");
		auto usercenter = wxXmlResource::Get()->LoadPanel(boptr, "usercenter");
		boptr->AddPage(mutipanel, "多人游戏");
		boptr->AddPage(gamepanel, "游戏大厅");
		boptr->AddPage(usercenter, "用户中心");
		auto pp = new match_data(0,0,false);
		auto ri = pa->resign_port(config::room_init_port);
		ri->OnMessage.connect([this,pp](lsy::buffer buf) {
			uint16_t flag;
			buf.get(flag);
			if (flag) {
				gui_run([this]() {
					auto boptr = XRCCTRL(*mf, "m_notebook1", wxNotebook);
					boptr->DeletePage(boptr->GetPageCount() - 1);
					res_dia
						= wxXmlResource::Get()->LoadDialog(mf, "game_result");
					res_dia->Show(true);
					mf->Refresh();
				});
				auto po=pa->resign_port(config::game_result);
				po->OnMessage.connect([this,flag=std::make_shared<uint8_t>(0)](lsy::buffer buf) {
					gui_run([buf_c=buf,flag, this]() {
						lsy::buffer buf(buf_c);
						if (*flag==0) {
							*flag =1;
							wxStaticText *st = XRCCTRL(*res_dia, "m_staticText134", wxStaticText);
							wxString str((char*)buf.get(0));
							st->SetLabelText(str);
							res_dia->Update();
						}
						else if(*flag == 1){
							*flag = 2;
							std::string name;
							std::string role;
							uint8_t is_win;
							int index = 0;
							while (buf.remain()) {
								buf.get(name);
								buf.get(role);
								buf.get(&is_win, 1);
								XRCCTRL(*res_dia, "m_listCtrl156", wxListCtrl)->InsertItem(index, wxString::Format("%d", index + 1));
								XRCCTRL(*res_dia, "m_listCtrl156", wxListCtrl)->SetItem(index,1,name);
								XRCCTRL(*res_dia, "m_listCtrl156", wxListCtrl)->SetItem(index, 2,role);
								XRCCTRL(*res_dia, "m_listCtrl156", wxListCtrl)->SetItem(index, 3, is_win ? "赢" : "输");
								res_dia->Update();
								index++;
							}
						}
						else {
							XRCCTRL(*res_dia, "m_textCtrl160", wxTextCtrl)->SetValue(buf.get(0));
						}
					});
				});
				po->start();
				po->write(lsy::buffer(size_t(0)), []() {});
			}
			else {
				std::get<2>(*pp) = false;
				gui_run([this]() {init_play(); });
			}
		});
		ri->write(lsy::buffer(size_t(0)), []() {});
		ri->start();

		auto mip = pa->resign_port(config::muti_info_port);
		mip->OnMessage.connect([flag=std::make_shared<bool>(true)](lsy::buffer buf) {
			gui_run([flag,buf]() {
				printf("listbox respone\n");
				if (*flag) {
					printf("%s\n", buf.get(0));
					XRCCTRL(*mf, "m_staticText4", wxStaticText)->SetLabel(buf.get(0));
					*flag = false;
				}
				else {
					uint16_t m;
					buf.get(m);
					printf("战斗力：%d\n", m);
					XRCCTRL(*mf, "m_staticText226", wxStaticText)->SetLabel(wxString::Format("战斗力：%d", m));
					*flag = true;
				}
				mf->Refresh();
			});
				
		});
		mip->start();
		wxListBox * mlb1 = XRCCTRL(*mf, "m_listBox1", wxListBox);
		mlb1->Bind(wxEVT_LISTBOX, [mlb1,this](auto it) {
			std::string str=mlb1->GetString(mlb1->GetSelection()).ToStdString();
			pa->ports[config::muti_info_port]->write(str, []() {});
		});
		auto mp = pa->resign_port(config::match_port);
		mp->OnMessage.connect([pp, this](lsy::buffer buf) {
				buf.get(std::get<1>(*pp));
				gui_run([pp, this]() {
					set_text(XRCCTRL(*mf, "m_staticText3", wxStaticText), pp);
				});
		});
		mp->start();
		auto msp = pa->resign_port(config::match_status_port);
		msp->OnMessage.connect([pp, this](lsy::buffer buf) {
			buf.get(std::get<0>(*pp));
			gui_run([pp, this]() {
				set_text(XRCCTRL(*mf, "m_staticText3", wxStaticText), pp);
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
		XRCCTRL(*mf,"m_button4", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this,pp](wxCommandEvent &event) {
			if (std::get<2>(*pp) == false) {
				std::get<2>(*pp) = true;
				wxListBox *lb = XRCCTRL(*mf, "m_listBox1", wxListBox);
				wxString str = lb->GetString(lb->GetSelection());
				pa->ports[config::match_port]->write(str.ToStdString(), []() {});
			}
		});

		auto gp = pa->resign_port(config::game_info_port);
		gp->OnMessage.connect([flag = std::make_shared<bool>(true)](lsy::buffer buf) {
			gui_run([flag, buf]() {
				printf("%s\n", buf.get(0));
				XRCCTRL(*mf, "m_staticText319", wxStaticText)->SetLabel(buf.get(0));
				mf->Refresh();
			});

		});
		gp->start();
		wxListBox * mlb2 = XRCCTRL(*mf, "m_listBox224", wxListBox);
		mlb2->Bind(wxEVT_LISTBOX, [mlb2, this](auto it) {
			std::string str = mlb2->GetString(mlb2->GetSelection()).ToStdString();
			pa->ports[config::game_info_port]->write(str, []() {});
		});
		auto mnp = pa->resign_port(config::match_noscore_port);
		mnp->OnMessage.connect([pp, this](lsy::buffer buf) {
			buf.get(std::get<1>(*pp));
			gui_run([pp, this]() {
				set_text(XRCCTRL(*mf, "m_staticText222", wxStaticText), pp);
			});
		});
		mnp->start();
		auto mnsp = pa->resign_port(config::match_noscore_status_port);
		mnsp->OnMessage.connect([pp, this](lsy::buffer buf) {
			buf.get(std::get<0>(*pp));
			gui_run([pp, this]() {
				set_text(XRCCTRL(*mf, "m_staticText222", wxStaticText), pp);
			});
		});
		mnsp->start();
		po = pa->resign_port(config::games_port);
		po->OnMessage.connect([this, po = lsy::as_ptr< lsy::port >(po)](
			lsy::buffer buf) {
			if (buf.size() == 1)
			{
				wxStaticCast(
					wxWindow::FindWindowByName("m_staticText516", mf),
					wxStaticText)
					->SetLabel("加载完毕");
				mf->Refresh();
			}
			else
			{
				wxStaticCast(wxWindow::FindWindowByName("m_listBox224", mf),
					wxListBox)
					->Append(wxString((char*)buf.data()));
				mf->Refresh();
			}
		});
		po->start();

		func = [this,pp](auto a) {
				pa->ports[config::games_port]->write(lsy::buffer((size_t)0), []() {});
				XRCCTRL(*mf, "m_listBox224", wxListBox)->Clear();
				XRCCTRL(*mf, "m_staticText516", wxStaticText)->SetLabel("加载中");
				mf->Refresh();
		};
		XRCCTRL(*mf, "m_button515", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, func);
		func(wxCommandEvent());

		XRCCTRL(*mf, "m_button422", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this,pp](wxCommandEvent &event) {
			if (std::get<2>(*pp) == false) {
				std::get<2>(*pp) = true;
			wxListBox *lb = XRCCTRL(*mf, "m_listBox224", wxListBox);
			printf("button click\n");
			wxString str = lb->GetString(lb->GetSelection());
				pa->ports[config::match_noscore_port]->write(str.ToStdString(), []() {});
			}
			
		});
		pa->resign_port(config::login_comfirm_port)->write(lsy::buffer(size_t(0)),[]() {});
		pa->ports[config::match_noscore_port]->write("lrs"s, []() {});
		auto pcp = pa->resign_port(config::passwd_change_port);
		pcp->OnMessage.connect([this](lsy::buffer buf) {
			uint16_t f;
			buf.get(f);
			gui_run([this,f]() {
				XRCCTRL(*mf, "m_staticText280", wxStaticText)->SetLabel(f ? "修改成功" : "旧密码错误"); 
				mf->Refresh();
			});
			
		});
		pcp->start();
		XRCCTRL(*mf, "m_panel435", wxPanel)->Show(false);
		XRCCTRL(*mf, "m_button429", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](auto &a) {
			XRCCTRL(*mf, "m_panel435", wxPanel)->Show(true);
		});
		XRCCTRL(*mf, "m_button282", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](auto &a) {
			if (XRCCTRL(*mf, "m_textCtrl266", wxTextCtrl)->GetLineLength(0) != 0
				&& XRCCTRL(*mf, "m_textCtrl270", wxTextCtrl)->GetLineLength(0)
				!= 0)
			{
				if (XRCCTRL(*mf, "m_textCtrl278", wxTextCtrl)->GetValue() == XRCCTRL(*mf, "m_textCtrl270", wxTextCtrl)->GetValue()) {
					std::string oldps = XRCCTRL(*mf, "m_textCtrl266", wxTextCtrl)->GetValue();
					std::string newps = XRCCTRL(*mf, "m_textCtrl270", wxTextCtrl)->GetValue();
					lsy::buffer buf(oldps.size() + 2 + newps.size());
					buf.put(oldps);
					buf.put(newps);
					pa->ports[config::passwd_change_port]->write(buf, []() {});
				}
				else {
					XRCCTRL(*mf, "m_staticText280", wxStaticText)->SetLabel("请输出相同的密码");
				}
			}
			else {
				XRCCTRL(*mf, "m_staticText280", wxStaticText)->SetLabel("不可为空");
			}
		});
		
		pa->resign_port(config::player_rule_create_port)->start();
		pa->resign_port(config::player_role_create_port)->start();
		XRCCTRL(*mf, "m_button292", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](auto &a) {

			if (XRCCTRL(*mf, "m_filePicker286", wxFilePickerCtrl)->GetPath().size() != 0
				&& XRCCTRL(*mf, "m_textCtrl300", wxTextCtrl)->GetLineLength(0)
				!= 0 && XRCCTRL(*mf, "m_choice288", wxChoice)->GetSelection()!= wxNOT_FOUND)
			{
				auto port = XRCCTRL(*mf, "m_choice288", wxChoice)->GetSelection() == 1 ? config::player_role_create_port : config::player_rule_create_port;
				std::string name = XRCCTRL(*mf, "m_textCtrl300", wxTextCtrl)->GetValue();
				std::string file = XRCCTRL(*mf, "m_filePicker286", wxFilePickerCtrl)->GetPath();
				FILE *fp = fopen(file.c_str(), "r");
				fseek(fp, 0, 2);
				size_t size = ftell(fp);
				void *p=malloc(size);
				fseek(fp, 0, 0);
				size=fread((char *)p, 1, size, fp);
				lsy::buffer buf(name.size() + 1 + size);
				buf.put(name);
				buf.put((unsigned char*)p, size);
				pa->ports[port]->write(buf, []() {});
				pa->ports[config::player_rule_get_port]->write(lsy::buffer(size_t(0)), []() {});
				pa->ports[config::player_role_get_port]->write(lsy::buffer(size_t(0)), []() {});
				XRCCTRL(*mf, "m_choice288", wxChoice)->SetSelection(-1);
				XRCCTRL(*mf, "m_textCtrl300", wxTextCtrl)->Clear();
				XRCCTRL(*mf, "m_filePicker286", wxFilePickerCtrl)->SetPath("");
				XRCCTRL(*mf, "m_panel435", wxPanel)->Show(false);
			}
			else {
			}
		});

		pa->resign_port(config::player_room_create_port)->start();
		pa->resign_port(config::player_room_delete_port)->start();
		XRCCTRL(*mf, "m_grid332", wxGrid)->CreateGrid(0,2, wxGrid::wxGridSelectRows);
		XRCCTRL(*mf, "m_grid332", wxGrid)->SetColLabelValue(0,"角色名");
		XRCCTRL(*mf, "m_grid332", wxGrid)->SetColLabelValue(1, "角色数量");
		XRCCTRL(*mf, "m_button340", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](auto &a) {
			XRCCTRL(*mf, "m_grid332", wxGrid)->AppendRows();
		});
		XRCCTRL(*mf, "m_button342", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](auto &a) {
			XRCCTRL(*mf, "m_grid332", wxGrid)->DeleteRows(XRCCTRL(*mf, "m_grid332", wxGrid)->GetSelectedRows().front());
		});
		XRCCTRL(*mf, "m_button350", wxButton)->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](auto &a) {

			if (XRCCTRL(*mf, "m_textCtrl348", wxTextCtrl)->GetLineLength(0) != 0
				&& XRCCTRL(*mf, "m_textCtrl388", wxTextCtrl)->GetLineLength(0))
			{
				std::string room_name = XRCCTRL(*mf, "m_textCtrl348", wxTextCtrl)->GetValue();
				std::string rule_name = XRCCTRL(*mf, "m_textCtrl388", wxTextCtrl)->GetValue();
				auto table = XRCCTRL(*mf, "m_grid332", wxGrid)->GetTable();
				size_t size = room_name.size() + rule_name.size() + 4;
				int n= table->GetRowsCount();
				for (int i = 0; i < n; i++) {
					size += table->GetValue(i,0).size()+1;
					size += 2;
				}
				lsy::buffer buf(size);
				buf.put(room_name);
				buf.put(rule_name);
				buf.put((uint16_t)n);
				for (int i = 0; i < n; i++) {
					buf.put(table->GetValue(i, 0).ToStdString());
					auto na = boost::lexical_cast<uint16_t>(table->GetValue(i, 1).ToStdString());
					buf.put(na);
				}
				int m=buf.readed();
				pa->ports[config::player_room_delete_port]->write(room_name, []() {});
				pa->ports[config::player_room_create_port]->write(buf, []() {});
				pa->ports[config::player_room_get_port]->write(""s, []() {});
			}
		});
		
		auto player_role_get_port =pa->resign_port(config::player_role_get_port);
		player_role_get_port->OnMessage.connect([flag=std::make_shared<int>(0)](auto buf) {
			if (buf.size() == 0) {
				*flag = 0;
			}
			else {
				if (*flag == 0) {
					XRCCTRL(*mf, "m_listBox425", wxListBox)->Clear();
				}
				std::string str;
				buf.get(str);
				wxString st = str;
				XRCCTRL(*mf, "m_listBox425", wxListBox)->InsertItems(1, &st, *flag);
				++*flag;
			}
		});
		player_role_get_port->start();

		auto player_rule_get_port = pa->resign_port(config::player_rule_get_port);
		player_rule_get_port->OnMessage.connect([flag = std::make_shared<int>(0)](auto buf) {
			if (buf.size() == 0) {
				*flag = 0;
			}
			else {
				if (*flag == 0) {
					XRCCTRL(*mf, "m_listBox427", wxListBox)->Clear();
				}
				std::string str;
				buf.get(str);
				wxString st = str;
				XRCCTRL(*mf, "m_listBox427", wxListBox)->InsertItems(1, &st, *flag);
				++*flag;
			}
		});
		player_rule_get_port->start();

		auto player_room_detail_port=pa->resign_port(config::player_room_detail_port);
		player_room_detail_port->OnMessage.connect([this](lsy::buffer buf) {
			gui_run([this,buf_=buf]() {
				auto tab = XRCCTRL(*mf, "m_grid332", wxGrid)->GetTable();
				if(tab->GetRowsCount())
					tab->DeleteRows(0,tab->GetRowsCount());
				lsy::buffer buf = buf_;
				std::string room_name;
				std::string rule_name;
				uint16_t size;
				buf.get(room_name);
				XRCCTRL(*mf, "m_textCtrl348", wxTextCtrl)->SetValue(room_name);
				buf.get(rule_name);
				XRCCTRL(*mf, "m_textCtrl388", wxTextCtrl)->SetValue(rule_name);
				buf.get(size);
				while (size--) {
					uint16_t count;
					std::string role_name;
					buf.get(role_name);
					buf.get(count);
					tab->AppendRows();
					int num=tab->GetRowsCount();
					tab->SetValue(num-1,0, role_name);
					tab->SetValue(num - 1, 1, wxString::Format("%d", count));
				}
			});
		});
		player_room_detail_port->start();
		auto mlb4 = XRCCTRL(*mf, "m_listBox413", wxListBox);
		mlb4->Bind(wxEVT_LISTBOX, [mlb4, this](auto it) {
			std::string str = mlb4->GetString(mlb4->GetSelection()).ToStdString();
			pa->ports[config::player_room_detail_port]->write(str, []() {});
		});
		auto player_room_get_port = pa->resign_port(config::player_room_get_port);
		player_room_get_port->OnMessage.connect([flag = std::make_shared<int>(0)](auto buf) {
			if (buf.size() == 0) {
				*flag = 0;
			}
			else {
				if (*flag == 0) {
					XRCCTRL(*mf, "m_listBox413", wxListBox)->Clear();
				}
				std::string str;
				buf.get(str);
				wxString st = str;
				XRCCTRL(*mf, "m_listBox413", wxListBox)->InsertItems(1,&st,*flag);
				++*flag;
			}
		});
		player_room_get_port->start();

		pa->ports[config::player_room_get_port]->write(lsy::buffer(size_t(0)), []() {});
		pa->ports[config::player_rule_get_port]->write(lsy::buffer(size_t(0)), []() {});
		pa->ports[config::player_role_get_port]->write(lsy::buffer(size_t(0)), []() {});
}
void DerivedApp::init_play()
{
	auto boptr = XRCCTRL(*mf, "m_notebook1", wxNotebook);
	auto playerpanel
		= wxXmlResource::Get()->LoadPanel(boptr, "playpanel");
	boptr->AddPage(playerpanel, "游戏", true);
	auto rb = XRCCTRL(*mf, "m_ribbonBar5", wxRibbonBar);
	rb->Bind(wxEVT_RIBBONBAR_PAGE_CHANGED, [this](wxRibbonBarEvent e) {
		current_channel= wxStaticCast(e.GetPage()->GetChildren().back(),text_panel);
	});
	auto ro_info = pa->resign_port(config::room_info);
	ro_info->OnMessage.connect([this, playerpanel](auto buf) {
		std::string str((char*)buf.get(0));
		buf.get(str.size() + 1);
		uint8_t num;
		uint8_t index;
		buf.get(&num, 1);
		buf.get(&index, 1);
		printf("try init\n");
		gui_run([this, num, str, index, playerpanel]() {
			printf("start init\n");
			auto left = XRCCTRL(*mf, "m_ribbonPage112", wxRibbonPage);
			auto right = XRCCTRL(*mf, "m_ribbonPage114", wxRibbonPage);
			player_pannel = new pl_panel(left, right, num);
			printf("player_pannel inited%p,%p\n", player_pannel,&player_pannel);
			left->Realize();
			right->Realize();
			XRCCTRL(*mf, "m_staticText28", wxStaticText)->SetLabel(wxString::Format("你的身份是：%s\n你的位置是：%d\n", str, index + 1));
			playerpanel->Refresh();
		});

		auto p_ch = pa->resign_port(config::public_channel);
		p_ch->OnMessage.connect([this](auto buf) {
			XRCCTRL(*mf, "m_textCtrl30", wxTextCtrl)->AppendText((char*)buf.get(0));
			mf->Refresh();
		});
		p_ch->start();


		auto rl_po = pa->resign_port(config::role_list);
		rl_po->OnMessage.connect([this](auto buf) {
			uint8_t is_show;
			uint8_t index;
			buf.get(is_show);
			buf.get(index);
			if (is_show) {
				std::string name;
				buf.get(name);
				gui_run([name, index, this]() {
					player_pannel->pannels[index]->set_role(name);
				});
			}
			else {
				gui_run([ index, this]() {
					player_pannel->pannels[index]->clear_role();
				});
			}
		});
		rl_po->start();

		auto ch_con = pa->resign_port(config::channel_open);
		ch_con->OnMessage.connect([playerpanel, this, ch_con](auto buf) {
			uint16_t op;
			buf.get(op);
			std::string name((char *)buf.get(buf.remain()));
			if (channel_map.find(name) == channel_map.end())
			{
				channel_map[name] = 0;
				gui_run([name, playerpanel, this, op,ch_con]() {
					wxRibbonBar *bar = XRCCTRL(*mf, "m_ribbonBar5", wxRibbonBar);
					auto rp = new wxRibbonPage(bar, wxID_ANY, name);
					auto po = pa->resign_port(op);
					auto tp = new text_panel(po, rp);
					current_channel = tp;
					channel_map[name] = tp;
					XRCCTRL(*mf, "m_ribbonBar5", wxRibbonBar)->Realize();
					mf->Maximize(false);
					mf->Maximize(true);
					ch_con->write(lsy::buffer(size_t(0)), []() {});
				});
			}
			else {
				printf("warning muti create channel\n");
			}
		});
		ch_con->start();

		auto ch_en = pa->resign_port(config::channel_enable);
		ch_en->OnMessage.connect([this](auto buf) {
			uint16_t b;
			buf.get(b);
			std::string name((char*)buf.get(1));
			gui_run([this, name, b]() {
				if (channel_map.find(name) != channel_map.end())
				{
					channel_map[name]->Enable(b);
					mf->Refresh();
				}
				else {
					printf("skip:%s\n", name.c_str());
				}
			});
		});
		ch_en->start();


		auto bupo = pa->resign_port(config::button_port);
		bupo->OnMessage.connect([this](auto buf) {
			std::string name;
			uint16_t port;
			buf.get(port);
			buf.get(name);
			auto buindex = std::make_shared<std::vector<uint8_t>>(buf.begin()+buf.readed(),buf.end());
			if (buttons.find(name) == buttons.end()) {
				buttons.insert(name);
				auto po = pa->resign_port(port);
				po->start();
				po->OnMessage.connect([name, po, this, buindex](auto buf) {
					buttons.erase(name);
					gui_run([name, po, this, buindex]() {
						for (auto index : *buindex) {
							player_pannel->pannels[index]->remove(name);
						}
						XRCCTRL(*mf, "m_ribbonPage112", wxRibbonPage)->Realize();
						XRCCTRL(*mf, "m_ribbonPage114", wxRibbonPage)->Realize();
						mf->Refresh();
						po->close();
					});
				});
				gui_run([name, po, this, buindex]() {
					uint8_t index = 0;
					for (auto index : *buindex) {
						player_pannel->pannels[index]->add(name, [po, index]() {
							lsy::buffer buf(size_t(1));
							buf.put(&index, 1);
							po->write(buf, []() {});
						});
					}
					XRCCTRL(*mf, "m_ribbonPage112", wxRibbonPage)->Realize();
					XRCCTRL(*mf, "m_ribbonPage114", wxRibbonPage)->Realize();
					mf->Refresh();
				});
			}
			else {
				printf("create muti same button\n");
			}
		});
		bupo->start();

		auto pl_de = pa->resign_port(config::player_dead);
		pl_de->OnMessage.connect([this](auto buf) {
			uint8_t a;
			uint8_t b;
			buf.get(&a, 1);
			buf.get(&b, 1);
			gui_run([a, b, this]() {
				player_pannel->set_status(a, b);
				mf->Refresh();
			});
		});
		pl_de->start();

		auto ch_cl = pa->resign_port(config::channel_close);
		ch_cl->OnMessage.connect([this](auto buf) {
			printf("in:%d\n", buf.size());
			std::string name((char*)buf.get(0));
			auto it = channel_map.find(name);
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
		pa->ports[config::room_init_port]->write(uint16_t(1), []() {});
	});
	ro_info->start();
	pa->ports[config::room_init_port]->write(uint16_t(0), []() {});
	mf->Layout();
	mf->Refresh();
	mf->Show();
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
	auto func= [this](wxCommandEvent a) {
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
	};
	XRCCTRL(*dlg, "login_b", wxButton)
		->Bind(wxEVT_COMMAND_BUTTON_CLICKED, func);
    dlg->SetTitle("connecting");
    dlg->Show();
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml("client.xml", pt);
    li.OnConnect.connect([this, func](auto port) {
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
				case 3:
					dlg->SetTitle("already login!");
					dlg->Refresh();
					break;
                default:
                    assert(0);
                    break;
            }
        });
        p->OnDestroy.connect([this]() { gui_run([this]() { dlg->Close(); }); });
        gui_run([this,func]() {
            dlg->SetTitle("connected");
            dlg->Refresh();
            XRCCTRL(*dlg, "login_b", wxButton)->Enable(true);
			if (wxGetApp().argc >= 3) {
				XRCCTRL(*dlg, "id_tc", wxTextCtrl)->SetValue(wxGetApp().argv[1]);
				XRCCTRL(*dlg, "passwd_tc", wxTextCtrl)->SetValue(wxGetApp().argv[2]);
				func(wxCommandEvent());
			}
        });
    });
    li.add_group(pt.find("client")->second);/**/
    return true;
}
