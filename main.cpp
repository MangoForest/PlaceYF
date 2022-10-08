#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/timer.hpp>
#include "PlaceYF.h"

using namespace nana;

int main() {
    if(!PlaceYF::Dll_Injection("libHouseMemory.dll","ffxiv_dx11.exe")){
        return 0;
    };
    PlaceYF place_client("127.0.0.1:8932");
    form fm( API::make_center(400, 180),appearance(true, true, true, false, false, false, false));
    fm.icon(paint::image("64.ico"));
    fm.caption("PlaceYF");
    fm.bgcolor(color(0xe8,0xee,0xf9));

    place layout(fm);
    layout.div("vert<><<enforce margin=[0,20,0,20]><>><cur margin=[0,20,0,20]><to margin=[0,20,0,20]><func margin=[0,20,0,20]><>");

    label lb_placeanywhere(fm,"解除限制");
    checkbox ck_placeanywhere(fm);
    label lb_curpos(fm,"当前位置");
    label lb_curx(fm,"0.0");
    label lb_cury(fm,"0.0");
    label lb_curz(fm,"0.0");
    label lb_currot(fm,"0.0");
    label lb_toplace(fm,"放置位置");
    textbox lb_toplacex(fm);
    textbox lb_toplacey(fm);
    textbox lb_toplacez(fm);
    textbox lb_toplacerot(fm);
    button btn_place(fm,"放置");
    button btn_sync_place(fm,"复制");

    ck_placeanywhere.bgcolor(color(0xe8,0xee,0xf9));


    lb_toplacex.multi_lines(false);
    lb_toplacey.multi_lines(false);
    lb_toplacez.multi_lines(false);
    lb_toplacerot.multi_lines(false);

    layout["enforce"]<<lb_placeanywhere<<ck_placeanywhere;
    layout["cur"]<<lb_curpos<<lb_curx<<lb_cury<<lb_curz<<lb_currot;
    layout["to"]<<lb_toplace<<lb_toplacex<<lb_toplacey<<lb_toplacez<<lb_toplacerot;
    layout["func"]<<btn_place<<btn_sync_place;
    layout.collocate();

    lb_toplacex.set_accept([](wchar_t key){
        if(('0'<=key&&key<='9')||key=='-'||key=='.'||key==8){
            return true;
        }else{
            return false;
        }
    });
    lb_toplacey.set_accept([](wchar_t key){
        if(('0'<=key&&key<='9')||key=='-'||key=='.'||key==8){
            return true;
        }else{
            return false;
        }
    });
    lb_toplacez.set_accept([](wchar_t key){
        if(('0'<=key&&key<='9')||key=='-'||key=='.'||key==8){
            return true;
        }else{
            return false;
        }
    });
    lb_toplacerot.set_accept([](wchar_t key){
        if(('0'<=key&&key<='9')||key=='-'||key=='.'||key==8){
            return true;
        }else{
            return false;
        }
    });

    ck_placeanywhere.events().checked([&place_client](const arg_checkbox& ei){
        if(ei.widget->checked()){
            place_client.setPlaceAnywhere(true);
        }else{
            place_client.setPlaceAnywhere(false);
        }
    });

    btn_place.events().click([&](const arg_click& ei){
        try {
            place_client.writePos(lb_toplacex.to_double(), lb_toplacey.to_double(), lb_toplacez.to_double(), lb_toplacerot.to_double());
        } catch (std::invalid_argument&e) {
            (msgbox(fm,"错误输入",msgbox::ok)<<"请输入数字").show();
        } catch (std::exception &e) {
            std::cout<<e.what()<<std::endl;
        }
    });

    btn_sync_place.events().click([&](const arg_click& ei){
        lb_toplacex.caption(lb_curx.caption());
        lb_toplacey.caption(lb_cury.caption());
        lb_toplacez.caption(lb_curz.caption());
        lb_toplacerot.caption(lb_currot.caption());
    });

    timer timer{std::chrono::milliseconds {50}};
    timer.elapse([&]{
        auto p=place_client.getPos();
        lb_curx.caption(std::to_string(p[0]));
        lb_cury.caption(std::to_string(p[1]));
        lb_curz.caption(std::to_string(p[2]));
        lb_currot.caption(std::to_string(p[3]));
    });
    timer.start();

    fm.show();
    exec();
    place_client.shutDown();
}

