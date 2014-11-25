/*
*	A Bedrock Platform-Independent Implementation
*	Nana C++ Library(http://www.nanapro.org)
*	Copyright(C) 2003-2014 Jinhao(cnjinhao@hotmail.com)
*
*	Distributed under the Boost Software License, Version 1.0.
*	(See accompanying file LICENSE_1_0.txt or copy at
*	http://www.boost.org/LICENSE_1_0.txt)
*
*	@file: nana/gui/detail/bedrock_pi.cpp
*/

#include <nana/config.hpp>

#include PLATFORM_SPEC_HPP
#include GUI_BEDROCK_HPP
#include <nana/gui/detail/event_code.hpp>
#include <nana/system/platform.hpp>
#include <sstream>
#include <nana/system/timepiece.hpp>
#include <nana/gui/wvl.hpp>
#include <nana/gui/detail/inner_fwd_implement.hpp>
#include <nana/gui/detail/native_window_interface.hpp>
#include <nana/gui/layout_utility.hpp>
#include <nana/gui/detail/element_store.hpp>

namespace nana
{
	namespace detail
	{
		void events_operation_register(event_handle evt)
		{
			bedrock::instance().evt_operation.register_evt(evt);
		}

		void events_operation_cancel(event_handle evt)
		{
			bedrock::instance().evt_operation.cancel(evt);
		}

		bool bedrock::_m_emit_core(event_code evt_code, core_window_t* wd, bool draw_only, const ::nana::detail::event_arg_interface& event_arg)
		{
			if (!this->wd_manager.available(wd))
				return false;

			switch (evt_code)
			{
			case event_code::click:
			case event_code::dbl_click:
			case event_code::mouse_enter:
			case event_code::mouse_move:
			case event_code::mouse_leave:
			case event_code::mouse_down:
			case event_code::mouse_up:
			{
				auto arg = dynamic_cast<const arg_mouse*>(&event_arg);
				if (nullptr == arg)
					return false;

				void(::nana::detail::drawer::*drawer_event_fn)(const arg_mouse&);
				::nana::basic_event<arg_mouse>* evt_addr;
				switch (evt_code)
				{
				case event_code::click:
					drawer_event_fn = &drawer::click;
					evt_addr = &wd->together.attached_events->click;
					break;
				case event_code::dbl_click:
					drawer_event_fn = &drawer::dbl_click;
					evt_addr = &wd->together.attached_events->dbl_click;
					break;
				case event_code::mouse_enter:
					drawer_event_fn = &drawer::mouse_enter;
					evt_addr = &wd->together.attached_events->mouse_enter;
					break;
				case event_code::mouse_move:
					drawer_event_fn = &drawer::mouse_move;
					evt_addr = &wd->together.attached_events->mouse_move;
					break;
				case event_code::mouse_leave:
					drawer_event_fn = &drawer::mouse_leave;
					evt_addr = &wd->together.attached_events->mouse_leave;
					break;
				case event_code::mouse_down:
					drawer_event_fn = &drawer::mouse_down;
					evt_addr = &wd->together.attached_events->mouse_down;
					break;
				case event_code::mouse_up:
					drawer_event_fn = &drawer::mouse_up;
					evt_addr = &wd->together.attached_events->mouse_up;
					break;
				default:
					throw std::runtime_error("Invalid mouse event code");
				}

				(wd->drawer.*drawer_event_fn)(*arg);
				if (!draw_only)
					evt_addr->emit(*arg);

				break;
			}
			case event_code::mouse_wheel:
			{
				auto arg = dynamic_cast<const arg_wheel*>(&event_arg);
				if (nullptr == arg)
					return false;

				wd->drawer.mouse_wheel(*arg);
				if (!draw_only)
					wd->together.attached_events->mouse_wheel.emit(*arg);
				break;
			}
			case event_code::key_press:
			case event_code::key_char:
			case event_code::key_release:
			case event_code::shortkey:
			{
				auto arg = dynamic_cast<const arg_keyboard*>(&event_arg);
				if (nullptr == arg)
					return false;

				void(::nana::detail::drawer::*drawer_event_fn)(const arg_keyboard&);
				::nana::basic_event<arg_keyboard>* evt_addr;

				switch (evt_code)
				{
				case event_code::key_press:
					drawer_event_fn = &drawer::key_press;
					evt_addr = &wd->together.attached_events->key_press;
					break;
				case event_code::key_char:
					drawer_event_fn = &drawer::key_char;
					evt_addr = &wd->together.attached_events->key_char;
					break;
				case event_code::key_release:
					drawer_event_fn = &drawer::key_release;
					evt_addr = &wd->together.attached_events->key_release;
					break;
				case event_code::shortkey:
					drawer_event_fn = &drawer::shortkey;
					evt_addr = &wd->together.attached_events->shortkey;
					break;
				default:
					throw std::runtime_error("Invalid keyboard event code");
				}
				(wd->drawer.*drawer_event_fn)(*arg);
				if (!draw_only)
					evt_addr->emit(*arg);
				break;
			}
			case event_code::expose:
				if (!draw_only)
				{
					auto arg = dynamic_cast<const arg_expose*>(&event_arg);
					if (nullptr == arg)
						return false;

					wd->together.attached_events->expose.emit(*arg);
				}
				break;
			case event_code::focus:
			{
				auto arg = dynamic_cast<const arg_focus*>(&event_arg);
				if (nullptr == arg)
					return false;

				wd->drawer.focus(*arg);
				if (!draw_only)
					wd->together.attached_events->focus.emit(*arg);
				break;
			}
			case event_code::move:
			{
				auto arg = dynamic_cast<const arg_move*>(&event_arg);
				if (nullptr == arg)
					return false;

				wd->drawer.move(*arg);
				if (!draw_only)
					wd->together.attached_events->move.emit(*arg);
				break;
			}
			case event_code::resizing:
			{
				auto arg = dynamic_cast<const arg_resizing*>(&event_arg);
				if (nullptr == arg)
					return false;

				wd->drawer.resizing(*arg);
				if (!draw_only)
					wd->together.attached_events->resizing.emit(*arg);
				break;
			}
			case event_code::resized:
			{
				auto arg = dynamic_cast<const arg_resized*>(&event_arg);
				if (nullptr == arg)
					return false;

				wd->drawer.resized(*arg);
				if (!draw_only)
					wd->together.attached_events->resized.emit(*arg);
				break;
			}
			case event_code::unload:
				if (!draw_only)
				{
					auto arg = dynamic_cast<const arg_unload*>(&event_arg);
					if ((nullptr == arg) || (wd->other.category != category::flags::root))
						return false;

					auto evt_ptr = dynamic_cast<events_root_extension*>(wd->together.attached_events);
					if (nullptr == evt_ptr)
						return false;

					evt_ptr->unload.emit(*arg);
				}
				break;
			case event_code::destroy:
				if (!draw_only)
				{
					auto arg = dynamic_cast<const arg_destroy*>(&event_arg);
					if (nullptr == arg)
						return false;

					wd->together.attached_events->destroy.emit(*arg);
				}
				break;
			default:
				throw std::runtime_error("Invalid event code");
			}
			return true;
		}
	}//end namespace detail
}//end namespace nana