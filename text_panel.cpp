///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "text_panel.h"
#include <functional>
typedef std::function< void() > func_type;
void gui_run(func_type func);

///////////////////////////////////////////////////////////////////////////

void text_panel::add(uint8_t index, std::string mess)
{
    if (index >= sqli_mes.size())
    {
        sqli_mes.resize(index + 1);
    }
    if (index == 0xff)
    {
        all << "public"
            << ":" << mess << "\n";
    }
    else
    {
        all << (uint16_t)index + 1 << ":" << mess << "\n";
    }
    sqli_mes[index] << "\n" << mess;
}

void text_panel::show_type(uint8_t index)
{
    if (index == 255)
    {
        printf("%s", all.str().c_str());
        m_textCtrl1->SetValue(all.str());
    }
    else
    {
        if (index >= sqli_mes.size())
        {
            sqli_mes.resize(index + 1);
        }
        m_textCtrl1->SetValue(sqli_mes[index].str());
    }
}

void text_panel::enable(bool flag)
{
    m_button1->Enable(flag);
    m_textCtrl2->Enable(flag);
}

text_panel::text_panel(lsy::port* po, wxWindow* parent, wxWindowID id,
                       const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
    , port(po)
    , this_count(0)
{
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);
    Bind(wxEVT_PAINT, [this, parent](auto& a) {
        if (parent)
            SetSize(parent->GetSize());
    });
    m_textCtrl1
        = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                         wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    bSizer1->Add(m_textCtrl1, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    m_textCtrl2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
    bSizer2->Add(m_textCtrl2, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

    m_button1 = new wxButton(this, wxID_ANY, wxT("发送"), wxDefaultPosition,
                             wxDefaultSize, 0);
    bSizer2->Add(m_button1, 0, wxALL | wxEXPAND, 5);

    bSizer1->Add(bSizer2, 0, wxEXPAND, 5);


    this->SetSizer(bSizer1);
    this->Layout();

    po->OnMessage.connect([this](lsy::buffer buf) {
        uint8_t  num = 0;
        uint32_t count;
        buf.get(count);
        if (count > this_count)
        {
            this_count = count;
            buf.get(&num, 1);
            gui_run([ str = std::string((char*)buf.get(0)), num, this ]() {
                add(num, str);
                show_type(255);
                Refresh();
                SetFocus();
            });
        }
        else
        {
            printf("skip the invalid message\n");
        }
    });
    po->start();
    m_button1->Bind(wxEVT_BUTTON, [this](auto ev) {
        std::string str = m_textCtrl2->GetValue();
        if (str.size() != 0)
        {
            m_textCtrl2->Clear();
            port->write(str, []() {});
            Refresh();
        }
    });
}

text_panel::~text_panel()
{
    port->close();
}
