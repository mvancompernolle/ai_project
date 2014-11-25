#include <sstream>

#include <ncurses/panel.hh>
#include <ncurses/pen.hh>
#include <ncurses/color.hh>

#include <ndk++/application.hh>
#include <ndk++/transport.hh>

struct at_mouse
{
	at_mouse(ncurses::panel& p): p_(p) {}
	ndk::event::result operator()(const ndk::mouse& ev)
	{
		ncurses::pen pn(p_);
		std::ostringstream ost;
		ost << "mouse event:" << "x=" << ev.x_ << ",y=" << ev.y_;
		ost << ",over=" << std::boolalpha << ndk::mouse::over(ev, p_) << std::endl;
		ost << ">>buttons:";
		ost << "1=" << std::boolalpha << ev.button1_;
		ost << ",2=" << std::boolalpha << ev.button2_;
		ost << ",3=" << std::boolalpha << ev.button3_ << std::endl;
		pn.write(ost.str());
		pn.commit();
		return ndk::event::accepted;
	}
	ncurses::panel& p_;
};

struct at_key
{
	at_key(ncurses::pen& p, ndk::engine& e): p_(p), e_(e) {}
	ndk::event::result operator()(const ndk::keyboard& ev)
	{
		std::ostringstream ost;
		ost << "keyboard event:'" << (char)ev.code_ << "' " << std::oct << ev.code_ << std::endl;
		p_.write(ost.str());
		if (ev.code_ == 'e') e_.emit(ndk::engine::enter_focus);
		if (ev.code_ == 'q') ndk::application::instance().quit();
		p_.commit();
		return ndk::event::accepted;
	}
	ncurses::pen& p_;
	ndk::engine& e_;
};

struct at_system
{
	at_system(ncurses::pen& p): p_(p) {}
	ndk::event::result operator()(const ndk::event& ev)
	{
		std::ostringstream ost;
		ost << "system event:" << ev.type_ << std::endl;
		p_.write(ost.str());
		p_.commit();
		return ndk::event::accepted;
	}
	ncurses::pen& p_;
};

int
main()
{
	ndk::application::obj_type &app = ndk::application::instance();

	ncurses::panel	win;
	ncurses::pen	pen(win);

	win.show();

	win.resize(2*app.width() / 3, 2*app.height() / 3);
	win.move((app.width() - win.width()) / 2,
		 (app.height() - win.height()) / 2);

	win.set_scroll(true);

	pen.fill(ncurses::color(COLOR_BLUE, COLOR_WHITE));
	pen.clear();
	pen.write("generate some events,press 'q' for quit\n");
	pen.write("press 'e' to test event emitting \n");

	ndk::engine	roadway(0, &win);

	roadway.on_mouse(at_mouse(win));
	roadway.on_keyboard(at_key(pen, roadway));
	roadway.on_system(at_system(pen));

	app.events()->in_focus(&roadway);

	app.exec();
}
