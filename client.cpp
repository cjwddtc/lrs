#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <listener.h>/*
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/wvl.hpp>
int main()
{
    using namespace nana;
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml("client.xml", pt);
    lsy::listener li;
    // li.OnConnect.connect([](){});
    li.add_group(pt.find("client")->second);
    // Define widgets
    form    fm;
    textbox usr{fm}, pswd{fm};
    button  login{fm, "Login"}, cancel{fm, "close"};
    login.enabled(false);
    label status(fm, "connecting server");
    usr.tip_string("User:").multi_lines(false);
    pswd.tip_string("Password:").multi_lines(false).mask('*');

    // Define a place for the form.
    place plc{fm};

    // Divide the form into fields
    plc.div(
        "<><weight=80% vertical<><weight=70% vertical <vertical gap=10 "
        "textboxs arrange=[25,25,25]>  <weight=25 gap=10 buttons> ><>><>");

    // Insert widgets

    // The field textboxs is vertical, it automatically adjusts the widgets' top
    // and height.
    plc.field("textboxs") << usr << pswd << status;

    plc["buttons"] << login << cancel;
    cancel.events().click([&li]() {
        li.close();
        API::exit();
    });
    login.events().click([&usr, &pswd]() {
        std::string id;
        std::string passwd;
        usr.getline(0, id);
        pswd.getline(0, passwd);
        std::cout << id << ":" << passwd << std::endl;
    });

    // Finially, the widgets should be collocated.
    // Do not miss this line, otherwise the widgets are not collocated
    // until the form is resized.
    plc.collocate();
    fm.show();
    exec();
    li.join();
}*/