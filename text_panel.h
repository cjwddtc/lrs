///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __TEXT_PANEL_H__
#define __TEXT_PANEL_H__

#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

///////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <vector>
///////////////////////////////////////////////////////////////////////////////
/// Class text_panel
///////////////////////////////////////////////////////////////////////////////
#include <port.h>
class text_panel : public wxPanel
{
  private:
  protected:
  public:
    wxTextCtrl*                      m_textCtrl1;
    wxTextCtrl*                      m_textCtrl2;
    lsy::port*                       port;
    wxButton*                        m_button1;
    std::stringstream                all;
    std::vector< std::stringstream > sqli_mes;
	uint32_t this_count;
    void add(uint8_t index, std::string mess);
    void show_type(uint8_t index);
    void enable(bool flag);
    text_panel(lsy::port* po, wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxPoint& pos   = wxDefaultPosition,
               const wxSize&  size  = wxSize(500, 300),
               long           style = wxTAB_TRAVERSAL);
    ~text_panel();
};

#endif //__TEXT_PANEL_H__
