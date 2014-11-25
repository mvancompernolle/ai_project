/* transport.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/

//for debug
#include <iostream>
#include <functional>

#include <ncurses/panel.hh>
#include <ncurses/assert.hh>

#include <ndk++/events.hh>
#include <ndk++/application.hh>

#include <curses.h>

#include <ndk++/transport.hh>

using namespace ndk;

namespace
{
	template <typename EventT>
	struct run_impl
	{ /**< functor to run callback functions */
		run_impl(const EventT& e): e_(e) {}

		template <typename CallbackT>
		event::result operator()(CallbackT& callback)
		{
			return callback(e_);
		}

		const EventT& e_;
	};

	template <typename EventT>
	run_impl<EventT> run_callback(const EventT &e){ return run_impl<EventT>(e); }

	template <typename Ret, typename Tp, typename Arg1T>
	struct mem_fun1_impl
	{ /**< simple helper binder of member function */
		mem_fun1_impl(Ret (Tp::*f)(Arg1T), Arg1T arg):
				f_(f),
				arg_(arg)
		{}

		Ret operator()(Tp *obj)
		{
			return (obj->*f_)(arg_);
		}
		Ret (Tp::*f_)(Arg1T);
		Arg1T arg_;
	};

	template <typename Ret, typename Tp, typename Arg1T, typename Arg2T>
	struct mem_fun2_impl
	{ /**< simple helper binder of member function */
		mem_fun2_impl(Ret (Tp::*f)(Arg1T, Arg2T), Arg1T arg1, Arg2T arg2):
				f_(f),
				arg1_(arg1),
				arg2_(arg2)
		{}

		Ret operator()(Tp *obj)
		{
			return (obj->*f_)(arg1_, arg2_);
		}
		Ret (Tp::*f_)(Arg1T, Arg2T);
		Arg1T arg1_;
		Arg2T arg2_;
	};
}


bool mouse::over(const mouse& ev, const ncurses::panel& p)
{
	if (p.hidden())
		return false;
	else
		return ((p.x() <= ev.x_) && (ev.x_ <= (p.x() + p.width() - 1)) &&
			(p.y() <= ev.y_) && (ev.y_ <= (p.y() + p.height() - 1)));
}

engine::engine(engine *parent, ncurses::panel* geometry):
		geometry_(geometry),
		in_focus_(this),
		loop_(false),
		accept_(true)
{
	parent_ = parent ? parent : application::instance().events(); //by default head is application
	parent_->add_child(this);
}

engine::engine():
		parent_(0),
		geometry_(0),
		in_focus_(this),
		loop_(false),
		accept_(true)
{}

engine::~engine()
{
	if (parent_) parent_->del_child(this);
}

void engine::in_focus(engine *item)
{
	if (std::find(childs_.begin(), childs_.end(), item) != childs_.end())
	{
		in_focus_ = item;
		in_focus_->system(event(enter_focus)); //notify item
	}
	else
		throw std::runtime_error("unable to set focus to unknown widget");
}

engine* engine::head() /**< @return main head of the transport tree */
{
	engine *head = this; //find local event loop head
	while (!head->loop_ && head->parent_) head = head->parent_;
	return head;
}

void engine::exec()
{
	//'cause I'm now in focus, notify itself about.
	enter_all();

	::update_panels(); //update current status
	::doupdate();

	loop_ = true;
	while (loop_) get(); //object in the focus generate next event
}

void engine::quit()
{
	loop_ = false;
	//break all local event loops of the childs_
	std::for_each(childs_.begin(), childs_.end(), std::mem_fun(&engine::quit));
}


void engine::on_keyboard(const keyboard_event& callback, bool front)
{
	if (front)
		keyboard_.push_front(callback);
	else
		keyboard_.push_back(callback);
}

void engine::on_mouse(const mouse_event& callback, bool front)
{
	if (front)
		mouse_.push_front(callback);
	else
		mouse_.push_back(callback);
}

void engine::on_system(const system_event& callback, bool front)
{
	if (front)
		system_.push_front(callback);
	else
		system_.push_back(callback);
}

void engine::rat(const mouse& ev)
{
	std::for_each(mouse_.begin(),
		      mouse_.end(),
		      run_callback(ev));
}

event::result engine::kbd(const keyboard& ev)
{
	event::result res = event::ignored;
	kbd_events::iterator it = std::find_if(keyboard_.begin(),
					       keyboard_.end(),
					       run_callback(ev));
	if (it != keyboard_.end())
	{
		res = event::accepted;
		std::for_each(++it,
			      keyboard_.end(),
			      run_callback(ev));
	}

	return res;
}

event::result engine::system(const event& ev)
{
	event::result res = event::ignored;
	sys_events::iterator it = std::find_if(system_.begin(),
					       system_.end(),
					       run_callback(ev));
	if (it != system_.end())
	{
		res = event::accepted;
		std::for_each(++it,
			      system_.end(),
			      run_callback(ev));
	}

	return res;
}

void engine::add_child(engine *p)
{
	childs_.push_back(p);
}

void engine::del_child(engine *p)
{
	childs_.erase(std::remove(childs_.begin(),
					  childs_.end(),
					  p), childs_.end());
}

void engine::little_switch(engine *item)
{
	if (item == in_focus_ || !item->accept()) return ; //nothing to switch

	//all permissions ok - let's switch focus now
	if (in_focus_ != this)
		in_focus_->leave_all();
	in_focus_ = item;
	if (in_focus_ != this)
		in_focus_->enter_all();

	::update_panels(); //update panel's status
}

void engine::big_switch(engine* item)
{
	child_items	list; //hierachly we need to walk to produce a big switch

	//first cycle determine widgets we need to switch
	for (;item != this;item = item->parent_) list.push_back(item);

	for (child_items::reverse_iterator it = list.rbegin();it != list.rend();it++)
		(*it)->parent_->little_switch(*it);
}

void engine::switch_to(engine* item)
{
	head()->big_switch(item);
}

void engine::find_over(const mouse& ev, child_items* list)
{
	if (geometry_ && mouse::over(ev, *geometry_)) list->push_back(this);

	std::for_each(childs_.begin(),
		      childs_.end(),
		      mem_fun2_impl<void, engine, const mouse&, child_items*>(&engine::find_over, ev, list));
}

void engine::process(const mouse& ev)
{
	//walk throw the all objects and find all objects under mouse pointer
	child_items	list;
	find_over(ev, &list);

	if (list.empty())
	{ //all lose it's focus
		leave_all();
		::update_panels();
		::doupdate();
		::beep();
		return ;
	}

	//then find widget over all visible widgets
	child_items::iterator top = std::max_element(list.begin(),
				    list.end(),
				    higher);

	big_switch(*top); //then made a big switch to top widget

	//let's widget process it's event
	//find latest widget who can process mouse event
	engine *current = *top;
	while (!current->accept() && current->parent_) current = current->parent_;
	if (current->parent_) current->rat(ev);

	//(*top)->rat(ev);//and let's top widget process event
}

void engine::find_leafs(child_items *list)
{
	child_items	active_childs;
	std::for_each(childs_.begin(),
		      childs_.end(),
		      mem_fun1_impl< void, engine, child_items* >(&engine::find_leafs, &active_childs));

	//if we cannot find active childs, but have active father
	//then add father if it's not hidden now
	if (active_childs.empty() && accept_ && geometry_ && !geometry_->hidden())
		list->push_back(this);
	else
		//append active childs to the big list
		list->insert(list->end(), active_childs.begin(), active_childs.end());
}

void engine::process(const keyboard& ev)
{
	//call at_event only for itself else call process event to pass messages
	//to subwidgets, last widget always have in_focus_ equals "this"
	engine *current = this;
	while (current->in_focus_ != current)
		current = current->in_focus_;
	if (current->kbd(ev) != event::accepted)
	{ // if widget ignore processing then we can process it itself
		if (9 == ev.code_)
		{
			child_items	list;
			find_leafs(&list); //collect all leaves of the transport subtree //FIXME: start search not from head, but from current in_focus

			if (list.empty()) return ; //nothing to switch to

			engine* next; //calculate next candidate to switch
			child_items::iterator it = std::find(list.begin(), list.end(), current);
			if (it == list.end()) //first item by default
				next = list.at(0);
			else
				next = (++it == list.end()) ? *list.begin() : *it;

			big_switch(next);
		}
	}
}

void engine::emit(int type)
{
	head()->process_system(event(type, this));
}

event::result engine::process_system(const event& ev)
{
	//find are some child accept this event
	child_items::const_iterator item =
		std::find_if(childs_.begin(),
			     childs_.end(),
			     mem_fun1_impl<event::result, engine, const event&>(&engine::process_system, ev));

	return ( (item != childs_.end()) ? event::accepted : system(ev) );
}

//send event to object and all it's childs
void engine::leave_all()
{
	system(event(leave_focus));
	std::for_each(childs_.begin(),
		      childs_.end(),
		      std::mem_fun(&engine::leave_all));
}

void engine::enter_all()
{
	engine *current = this;
	while (current->in_focus_ != current)
	{
		current->system(event(enter_focus));
		current = current->in_focus_;
	}
	current->system(event(enter_focus));
}

void engine::get()
{
	const int ch = ::wgetch(::stdscr);
        if (ch == ERR)
        {
            /* ignore a timeout condition. Hopefully the get() is being called
             * directly from the user and not from exec() as this will be an
             * unfortunate waste of processor time.
             * */
        }
#ifdef NCURSES_MOUSE_VERSION
        else if (ch == KEY_MOUSE)
	{

		MEVENT mev;
		NCURSES_ASSERT(::getmouse(&mev));

		mouse ev;
		ev.x_ = mev.x;
		ev.y_ = mev.y;
		ev.button1_ = ((mev.bstate & BUTTON1_CLICKED) == BUTTON1_CLICKED);
		ev.button2_ = ((mev.bstate & BUTTON2_CLICKED) == BUTTON2_CLICKED);
		ev.button3_ = ((mev.bstate & BUTTON3_CLICKED) == BUTTON3_CLICKED);

		process(ev);
	}
#endif
	else
	{
		process(keyboard(ch));
	}
	::doupdate(); //commit all changes to the screen
}

void engine::set_accept(bool val)
{
	accept_ = val;
}

bool engine::accept() const
{
	return accept_;
}
