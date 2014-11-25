#include <nana/gui/widgets/skeletons/text_editor.hpp>
#include <nana/gui/widgets/skeletons/textbase_export_interface.hpp>
#include <nana/system/dataexch.hpp>
#include <nana/unicode_bidi.hpp>

namespace nana{	namespace widgets
{
	namespace skeletons
	{
		textbase_event_agent_interface::~textbase_event_agent_interface()
		{}

		class text_editor::editor_behavior_interface
		{
		public:
			virtual ~editor_behavior_interface(){}

			/// Deletes lines between first and second, and then, second line will be merged into first line.
			virtual void merge_lines(std::size_t first, std::size_t second) = 0;
			//Calculates how many lines the specified line of text takes with a specified pixels of width.
			virtual void add_lines(std::size_t pos, std::size_t lines) = 0;
			virtual void pre_calc_line(std::size_t line, unsigned pixels) = 0;
			virtual void pre_calc_lines(unsigned pixels) = 0;
			virtual std::size_t take_lines() const = 0;
			/// Returns the number of lines that the line of text specified by pos takes.
			virtual std::size_t take_lines(std::size_t pos) const = 0;

			virtual void update_line(std::size_t textline, std::size_t secondary_before) = 0;
			virtual void render(nana::color_t fgcolor) = 0;
			virtual	nana::point		caret_to_screen(unsigned x, unsigned y) = 0;
			virtual nana::upoint	screen_to_caret(int x, int y) = 0;
			virtual bool move_caret_ns(bool to_north) = 0;
			virtual bool adjust_caret_into_screen() = 0;
		};

		
		class text_editor::behavior_normal
			: public editor_behavior_interface
		{
		public:
			behavior_normal(text_editor& editor)
				: editor_(editor)
			{}

			void merge_lines(std::size_t first, std::size_t second) override{}
			void add_lines(std::size_t pos, std::size_t lines) override{}
			void pre_calc_line(std::size_t, unsigned) override{}
			void pre_calc_lines(unsigned) override{}

			std::size_t take_lines() const override
			{
				return editor_.textbase_.lines();
			}

			std::size_t take_lines(std::size_t pos) const override
			{
				return 1;
			}

			void update_line(std::size_t textline, std::size_t secondary_before) override
			{
				int top = editor_._m_text_top_base() + static_cast<int>(editor_.line_height() * (textline - editor_.points_.offset.y));
				editor_.graph_.rectangle(editor_.text_area_.area.x, top, editor_.text_area_.area.width, editor_.line_height(), API::background(editor_.window_), true);
				editor_._m_draw_string(top, API::foreground(editor_.window_), nana::upoint(0, editor_.points_.caret.y), editor_.textbase_.getline(textline), true);
			}

			void render(nana::color_t fgcolor) override
			{
				auto & points = editor_.points_;

				std::size_t scrlines = editor_.screen_lines() + static_cast<unsigned>(points.offset.y);
				if (scrlines > editor_.textbase_.lines())
					scrlines = editor_.textbase_.lines();

				int y = editor_._m_text_top_base();
				const unsigned pixles = editor_.line_height();
				nana::upoint str_pos(0, static_cast<int>(points.offset.y));
				for (unsigned ln = points.offset.y; ln < scrlines; ++ln, y += pixles)
				{
					editor_._m_draw_string(y, fgcolor, str_pos, editor_.textbase_.getline(ln), true);
					++str_pos.y;
				}
			}

			nana::point	caret_to_screen(unsigned x, unsigned y) override
			{
				auto & textbase = editor_.textbase_;
				if (y > textbase.lines())	y = textbase.lines();

				x = editor_._m_pixels_by_char(textbase.getline(y), x) + editor_.text_area_.area.x;

				int pos_y = static_cast<int>((y - editor_.points_.offset.y) * editor_.line_height() + editor_._m_text_top_base());
				int pos_x = static_cast<int>(x - editor_.points_.offset.x);

				return nana::point(pos_x, pos_y);
			}

			nana::upoint screen_to_caret(int x, int y) override
			{
				const auto & textbase = editor_.textbase_;
				const auto & text_area = editor_.text_area_;
				auto & points = editor_.points_;

				nana::upoint res(0, static_cast<unsigned>(_m_textline_from_screen(y)));

				//Convert the screen point to text caret point
				const string_type& lnstr = textbase.getline(res.y);
				res.x = static_cast<int>(lnstr.size());
				if (res.x)
				{
					x += (points.offset.x - text_area.area.x);
					if (x > 0)
					{
						unicode_bidi bidi;
						std::vector<unicode_bidi::entity> reordered;
						bidi.linestr(lnstr.c_str(), lnstr.size(), reordered);

						std::size_t pxbuf_size = 0;
						std::unique_ptr<unsigned[]> pxbuf;

						int xbeg = 0;
						for (auto & ent : reordered)
						{
							std::size_t len = ent.end - ent.begin;
							unsigned str_w = editor_._m_text_extent_size(ent.begin, len).width;
							if (xbeg <= x && x < xbeg + static_cast<int>(str_w))
							{
								if (len > pxbuf_size)
								{
									pxbuf_size = len;
									pxbuf.reset(new unsigned[len]);
								}
								res.x = editor_._m_char_by_pixels(ent.begin, len, pxbuf.get(), static_cast<int>(str_w), x - xbeg, _m_is_right_text(ent));
								res.x += static_cast<unsigned>(ent.begin - lnstr.c_str());
								return res;
							}
							xbeg += static_cast<int>(str_w);
						}
					}
					else
						res.x = 0;
				}

				return res;
			}

			bool move_caret_ns(bool to_north) override
			{
				auto & points = editor_.points_;

				if (to_north)	//North
				{
					if (points.caret.y)
					{
						points.caret.x = static_cast<unsigned>(editor_.textbase_.getline(--points.caret.y).size());

						if (points.xpos < points.caret.x)
							points.caret.x = points.xpos;

						bool redraw_required = (static_cast<int>(points.caret.y) < points.offset.y);

						if (static_cast<unsigned>(points.offset.y) > points.caret.y)
							editor_._m_offset_y(static_cast<int>(points.caret.y));

						if (adjust_caret_into_screen())
							redraw_required = true;

						return redraw_required;
					}
				}
				else //South
				{
					if (points.caret.y + 1 < editor_.textbase_.lines())
					{
						points.caret.x = static_cast<unsigned>(editor_.textbase_.getline(++points.caret.y).size());

						if (points.xpos < points.caret.x)
							points.caret.x = points.xpos;

						return adjust_caret_into_screen();
					}
				}
				return false;
			}

			//adjust_caret_into_screen
			//@brief:	Adjust the text offset in order to moving caret into visible area if it is out of the visible area
			//@note:	the function assumes the points_.caret is correct
			bool adjust_caret_into_screen() override
			{
				const auto scrlines = editor_.screen_lines();
				if (0 == scrlines)
					return false;

				auto& points = editor_.points_;
				auto& textbase = editor_.textbase_;

				editor_._m_get_scrollbar_size();

				const auto delta_pixels = editor_._m_text_extent_size(STR("    ")).width;
				auto x = points.caret.x;
				const string_type& lnstr = textbase.getline(points.caret.y);

				if (x > lnstr.size()) x = static_cast<unsigned>(lnstr.size());

				unsigned text_w = editor_._m_pixels_by_char(textbase.getline(points.caret.y), x);

				unsigned area_w = editor_._m_text_area().width;

				bool adjusted = true;
				if (static_cast<int>(text_w) < points.offset.x)
				{
					points.offset.x = (text_w > delta_pixels ? text_w - delta_pixels : 0);
				}
				else if (area_w && (text_w >= points.offset.x + area_w))
					points.offset.x = text_w - area_w + 2;
				else
					adjusted = false;

				int value = points.offset.y;
				if (scrlines && (points.caret.y >= points.offset.y + scrlines))
				{
					value = static_cast<int>(points.caret.y - scrlines) + 1;
					adjusted = true;
				}
				else if (static_cast<int>(points.caret.y) < points.offset.y)
				{
					if (scrlines >= static_cast<unsigned>(points.offset.y))
						value = 0;
					else
						value = static_cast<int>(points.offset.y - scrlines);
					adjusted = true;
				}
				else if (points.offset.y && (textbase.lines() <= scrlines))
				{
					value = 0;
					adjusted = true;
				}

				editor_._m_offset_y(value);
				editor_._m_scrollbar();
				return adjusted;
			}
		private:
			std::size_t	_m_textline_from_screen(int y) const
			{
				const auto & textbase = editor_.textbase_;
				const auto & text_area = editor_.text_area_;
				auto & points = editor_.points_;

				if (textbase.lines())
				{
					if (y < static_cast<int>(text_area.area.y))
						y = points.offset.y ? points.offset.y - 1 : 0;
					else
						y = (y - static_cast<int>(text_area.area.y)) / static_cast<int>(editor_.line_height()) + points.offset.y;

					if (textbase.lines() <= static_cast<unsigned>(y))
						return textbase.lines() - 1;
					else
						return static_cast<std::size_t>(y);
				}

				return 0;
			}
		private:
			text_editor& editor_;
		}; //end class behavior_normal


		class text_editor::behavior_linewrapped
			: public text_editor::editor_behavior_interface
		{
			struct text_section
			{
				const nana::char_t* begin;
				const nana::char_t* end;
				unsigned pixels;

				text_section()
				{
					throw std::runtime_error("text_section default construction is forbidden.");
				}

				text_section(const nana::char_t* ptr, const nana::char_t* endptr)
					: begin(ptr), end(endptr)
				{}
			};

			struct line_metrics
			{
				std::size_t		take_lines;	//The number of lines that text of this line takes.
				std::vector<text_section>	line_sections;
			};
		public:
			behavior_linewrapped(text_editor& editor)
				: editor_(editor)
			{}

			void merge_lines(std::size_t first, std::size_t second) override
			{
				if (first > second)
					std::swap(first, second);

				if (second < linemtr_.size())
					linemtr_.erase(linemtr_.begin() + first + 1, linemtr_.begin() + second);

				pre_calc_line(first, editor_.width_pixels());

				//textbase is implement by using deque, and the linemtr holds the text pointers
				//If the textbase is changed, it will check the text pointers.
				std::size_t line = 0;
				for (auto & mtr: linemtr_)
				{
					const nana::string& linestr = editor_.textbase_.getline(line);
					const nana::char_t * p = mtr.line_sections.front().begin;
					if (p < linestr.data() || (linestr.data() + linestr.size() < p))
						pre_calc_line(line, editor_.width_pixels());

					++line;
				}
			}

			void add_lines(std::size_t pos, std::size_t lines) override
			{
				if (pos < linemtr_.size())
				{
					for (std::size_t i = 0; i < lines; ++i)
						linemtr_.insert(linemtr_.begin() + pos + i, line_metrics());

					//textbase is implement by using deque, and the linemtr holds the text pointers
					//If the textbase is changed, it will check the text pointers.
					std::size_t line = 0;
					for (auto & mtr : linemtr_)
					{
						if (line < pos || (pos + lines) <= line)
						{
							const nana::string& linestr = editor_.textbase_.getline(line);
							const nana::char_t * p = mtr.line_sections.front().begin;
							if (p < linestr.data() || (linestr.data() + linestr.size() < p))
								pre_calc_line(line, editor_.width_pixels());
						}
						++line;
					}
				}
			}

			void pre_calc_line(std::size_t line, unsigned pixels) override
			{
				const string_type& lnstr = editor_.textbase_.getline(line);
				if (lnstr.empty())
				{
					auto & mtr = linemtr_[line];
					mtr.line_sections.clear();
					mtr.line_sections.emplace_back(lnstr.data(), lnstr.data());
					mtr.line_sections.back().pixels = 0;
					mtr.take_lines = 1;
					return;
				}

				std::vector<text_section> sections;
				_m_text_section(lnstr, sections);

				std::vector<text_section> line_sections;

				unsigned text_px = 0;
				const nana::char_t * secondary_begin = nullptr;
				for (auto & ts : sections)
				{
					if (nullptr == secondary_begin)
						secondary_begin = ts.begin;

					const unsigned str_w = editor_._m_text_extent_size(ts.begin, ts.end - ts.begin).width;

					text_px += str_w;
					if (text_px > pixels)
					{
						if (text_px != str_w)
						{
							line_sections.push_back(text_section(secondary_begin, ts.begin));
							line_sections.back().pixels = text_px - str_w;
							text_px = str_w;
							secondary_begin = ts.begin;
						}

						if (str_w > pixels)	//Indicates the splitting of ts string
						{
							std::size_t len = ts.end - ts.begin;
							std::unique_ptr<unsigned[]> pxbuf(new unsigned[len]);
							editor_.graph_.glyph_pixels(ts.begin, len, pxbuf.get());

							auto pxptr = pxbuf.get();
							auto pxend = pxptr + len;

							secondary_begin = ts.begin;
							text_px = 0;
							for (auto pxi = pxptr; pxi != pxend; ++pxi)
							{
								text_px += *pxi;
								if (text_px < pixels)
									continue;

								const nana::char_t * endptr = ts.begin + (pxi - pxptr) + (text_px == pixels ? 1 : 0);
								line_sections.emplace_back(secondary_begin, endptr);
								line_sections.back().pixels = text_px - (text_px == pixels ? 0 : *pxi);
								secondary_begin = endptr;

								text_px = (text_px == pixels ? 0 : *pxi);
							}
						}
						continue;
					}
					else if (text_px == pixels)
					{
						line_sections.emplace_back(secondary_begin, ts.begin);
						line_sections.back().pixels = text_px - str_w;
						secondary_begin = ts.begin;
						text_px = str_w;
					}
				}

				auto & mtr = linemtr_[line];
				
				if (line_sections.size())
				{
					mtr.take_lines = line_sections.size();
					mtr.line_sections.swap(line_sections);
				}
				else
				{
					mtr.take_lines = 0;
					mtr.line_sections.clear();
				}

				if (secondary_begin)
				{
					mtr.line_sections.emplace_back(secondary_begin, sections.back().end);
					mtr.line_sections.back().pixels = text_px;
					++mtr.take_lines;
				}
			}

			void pre_calc_lines(unsigned pixels) override
			{
				const auto lines = editor_.textbase_.lines();
				linemtr_.resize(editor_.textbase_.lines());

				for (std::remove_const<decltype(lines)>::type i = 0; i < lines; ++i)
					pre_calc_line(i, pixels);
			}

			std::size_t take_lines() const
			{
				auto lines = 0;
				for (auto & mtr : linemtr_)
					lines += mtr.take_lines;

				return lines;
			}

			std::size_t take_lines(std::size_t pos) const
			{
				return (pos < linemtr_.size() ? linemtr_[pos].take_lines : 0);
			}

			void update_line(std::size_t textline, std::size_t secondary_before) override
			{
				if (take_lines(textline) == secondary_before)
				{
					int top = caret_to_screen(0, textline).y;

					const unsigned pixels = editor_.line_height();
					editor_.graph_.rectangle(editor_.text_area_.area.x, top, editor_.width_pixels(), pixels * secondary_before, API::background(editor_.window_), true);

					auto fgcolor = API::foreground(editor_.window_);
					auto text_ptr = editor_.textbase_.getline(textline).data();

					for (std::size_t pos = 0; pos < secondary_before; ++pos, top+=pixels)
					{
						auto & sct = linemtr_[textline].line_sections[pos];
						editor_._m_draw_string(top, fgcolor, nana::upoint(static_cast<unsigned>(sct.begin - text_ptr), editor_.points_.caret.y), nana::string(sct.begin, sct.end), true);
					}
				}
				else
					editor_.render(API::is_focus_window(editor_.window_));
			}

			void render(nana::color_t fgcolor) override
			{
				std::size_t scrlines = editor_.screen_lines();

				std::size_t secondary;
				auto primary = _m_textline_from_screen(0, secondary);
				if (primary >= linemtr_.size() || secondary >= linemtr_[primary].line_sections.size())
					return;

				nana::upoint str_pos(0, static_cast<unsigned>(primary));
				str_pos.x = static_cast<unsigned>(linemtr_[primary].line_sections[secondary].begin - editor_.textbase_.getline(primary).data());

				int top = editor_._m_text_top_base();
				const unsigned pixels = editor_.line_height();

				for (std::size_t pos = 0; pos < scrlines; ++pos, top += pixels)
				{
					if ((primary < linemtr_.size()) && (secondary < linemtr_[primary].line_sections.size()))
					{
						auto & mtr = linemtr_[primary];
						auto & section = mtr.line_sections[secondary];

						nana::string text(section.begin, section.end);
						editor_._m_draw_string(top, fgcolor, str_pos, text, true);
						++secondary;
						if (secondary >= mtr.line_sections.size())
						{
							++primary;
							secondary = 0;
							str_pos.x = 0;
							++str_pos.y;
						}
						else
							str_pos.x += static_cast<unsigned>(text.size());
					}
					else
						break;
				}
			}

			nana::point	caret_to_screen(unsigned x, unsigned y) override
			{
				const auto & mtr = linemtr_[y];

				std::size_t lines = 0;	//lines before the caret line;
				for (auto & v : linemtr_)
				{
					if (y)
					{
						lines += v.take_lines;
						--y;
					}
					else
						break;
				}

				nana::point scrpos;
				if (0 != x)
				{
					for (auto & sec : mtr.line_sections)
					{
						std::size_t chsize = sec.end - sec.begin;
						if (x < chsize)
						{
							scrpos.x = editor_._m_pixels_by_char(nana::string(sec.begin, sec.end), x);
							break;
						}
						else if (x == chsize)
						{
							scrpos.x = editor_._m_text_extent_size(nana::string(sec.begin, sec.end).data(), sec.end - sec.begin).width;
							break;
						}
						else
						{
							x -= static_cast<unsigned>(chsize);
							++lines;
						}
					}
				}
				else
					scrpos.x = 0;

				scrpos.x += editor_.text_area_.area.x;
				scrpos.y = editor_.text_area_.area.y + static_cast<int>((lines - editor_.points_.offset.y) * editor_.line_height());
				return scrpos;
			}

			nana::upoint screen_to_caret(int x, int y) override
			{
				std::size_t secondary;
				std::size_t primary = _m_textline_from_screen(y, secondary);

				auto & mtr = linemtr_[primary];
				if (mtr.line_sections.empty())
					return nana::upoint(0, primary);

				//First of all, find the text of secondary.
				auto str = mtr.line_sections[secondary];

				std::vector<unicode_bidi::entity> reordered;
				unicode_bidi bidi;
				bidi.linestr(str.begin, str.end - str.begin, reordered);

				std::size_t pxbuf_size = 0;
				std::unique_ptr<unsigned[]> pxbuf;

				nana::upoint res(static_cast<unsigned>(str.begin - mtr.line_sections.front().begin), static_cast<unsigned>(primary));
				x -= editor_.text_area_.area.x;
				if (x < 0)
					x = 0;

				int xbeg = 0;
				for (auto & ent : reordered)
				{
					std::size_t len = ent.end - ent.begin;
					unsigned str_w = editor_._m_text_extent_size(ent.begin, len).width;
					if (xbeg <= x && x < xbeg + static_cast<int>(str_w))
					{
						if (len > pxbuf_size)
						{
							pxbuf.reset(new unsigned[len]);
							pxbuf_size = len;
						}
						res.x += editor_._m_char_by_pixels(ent.begin, len, pxbuf.get(), static_cast<int>(str_w), x - xbeg, _m_is_right_text(ent));
						res.x += static_cast<unsigned>(ent.begin - str.begin);
						return res;
					}
					xbeg += static_cast<int>(str_w);
				}
				res.x = editor_.textbase_.getline(res.y).size();
				return res;
			}

			bool move_caret_ns(bool to_north) override
			{
				auto & points = editor_.points_;

				nana::upoint secondary_pos;
				_m_pos_secondary(points.caret, secondary_pos);

				if (to_north)	//North
				{
					if (0 == secondary_pos.y)
					{
						if (0 == points.caret.y)
							return false;

						--points.caret.y;
						secondary_pos.y = static_cast<unsigned>(take_lines(points.caret.y)) - 1;
					}
					else
						--secondary_pos.y;
				}
				else //South
				{
					++secondary_pos.y;
					if (secondary_pos.y >= take_lines(points.caret.y))
					{
						secondary_pos.y = 0;
						if (points.caret.y + 1 >= editor_.textbase_.lines())
							return false;

						++points.caret.y;
					}
				}

				_m_pos_from_secondary(points.caret.y, secondary_pos, points.caret.x);
				return adjust_caret_into_screen();
			}

			bool adjust_caret_into_screen() override
			{
				const auto scrlines = editor_.screen_lines();
				if (0 == scrlines)
					return false;

				auto & points = editor_.points_;
				editor_._m_get_scrollbar_size();

				std::size_t off_secondary;
				auto off_primary = _m_textline(points.offset.y, off_secondary);

				unsigned caret_secondary;
				{
					nana::upoint secondpos;
					_m_pos_secondary(points.caret, secondpos);
					caret_secondary = secondpos.y;
				}

				//Use the caret line for the offset line when caret is in front of current offset line.
				if (off_primary > points.caret.y || (off_primary == points.caret.y && (off_secondary > caret_secondary)))
				{
					//Use the line which was specified by points.caret for the first line.
					_m_set_offset_by_secondary(points.caret.y, caret_secondary);
					return true;
				}

				//Find the last screen line. If the text does not reach the bottom of screen,
				//do not adjust the offset line.
				nana::upoint bottom;	//x=primary, y=secondary
				if (false == _m_advance_secondary(off_primary, off_secondary, static_cast<int>(scrlines - 1), bottom))
					return false;

				//Do not adjust the offset line if the caret line does not reach the bottom line.
				if (points.caret.y < bottom.x || (points.caret.y == bottom.x && caret_secondary <= bottom.y))
					return false;

				_m_advance_secondary(points.caret.y, caret_secondary, -static_cast<int>(scrlines - 1), bottom);

				_m_set_offset_by_secondary(bottom.x, bottom.y);
				return true;
			}
		private:
			void _m_text_section(const nana::string& str, std::vector<text_section>& tsec)
			{
				if (str.empty())
				{
					tsec.emplace_back(str.data(), str.data());
					return;
				}
				const auto end = str.data() + str.size();

				const nana::char_t * word = nullptr;
				for (auto i = str.data(); i != end; ++i)
				{
					nana::char_t const ch = *i;

					//CKJ characters and whitespace
					if (' ' == ch || '\t' == ch || (0x4E00 <= ch && ch <= 0x9FCF))
					{
						if (word)	//Record the word.
						{
							tsec.emplace_back(word, i);
							word = nullptr;
						}

						tsec.emplace_back(i, i + 1);
						continue;
					}

					if (nullptr == word)
						word = i;
				}

				if(word)
					tsec.emplace_back(word, end);
			}

			void _m_set_offset_by_secondary(std::size_t primary, std::size_t secondary)
			{
				std::size_t lines = 0;
				
				for_each(linemtr_.begin(), linemtr_.begin() + primary, [&lines](const line_metrics& mtr) mutable
				{
					lines += mtr.take_lines;
				});

				editor_.points_.offset.y = static_cast<int>(lines + secondary);
			}

			bool _m_advance_secondary(std::size_t primary, std::size_t secondary, int distance, nana::upoint& new_sec)
			{
				if (primary >= linemtr_.size())
					return false;

				if (secondary >= linemtr_[primary].take_lines)
					return false;

				if (0 == distance)
				{
					new_sec.x = static_cast<unsigned>(primary);
					new_sec.y = static_cast<unsigned>(secondary);
					return true;
				}

				if (distance < 0)
				{
					std::size_t n = static_cast<std::size_t>(-distance);

					if (secondary > n)
					{
						new_sec.x = static_cast<unsigned>(primary);
						new_sec.y = static_cast<unsigned>(secondary - n);
						return true;
					}

					if (0 == primary)
						return false;

					--primary;
					n -= (secondary + 1);

					while (true)
					{
						auto lines = linemtr_[primary].take_lines;
						if (lines >= n)
						{
							new_sec.x = static_cast<unsigned>(primary);
							new_sec.y = static_cast<unsigned>(lines - n);
							return true;
						}
						if (0 == primary)
							return false;

						--primary;
						n -= lines;
					}
				}
				else
				{
					std::size_t n = static_cast<std::size_t>(distance);

					if (linemtr_[primary].take_lines - (secondary + 1) >= n)
					{
						new_sec.x = static_cast<unsigned>(primary);
						new_sec.y = static_cast<unsigned>(secondary + n);
						return true;
					}

					n -= (linemtr_[primary].take_lines - (secondary + 1));
					++primary;
					while (primary < linemtr_.size())
					{
						auto & mtr = linemtr_[primary];
						if (mtr.take_lines >= n)
						{
							new_sec.x = static_cast<unsigned>(primary);
							new_sec.y = static_cast<unsigned>(n - 1);
							return true;
						}
						++primary;
						n -= mtr.take_lines;
					}
				}
				return false;
			}

			bool _m_pos_from_secondary(std::size_t textline, const nana::upoint& secondary, unsigned & pos)
			{
				if (textline >= linemtr_.size())
					return false;

				auto & mtr = linemtr_[textline];
				if (secondary.y >= mtr.line_sections.size() || mtr.line_sections.size() != mtr.take_lines)
					return false;

				auto & section = mtr.line_sections[secondary.y];
				unsigned len = static_cast<unsigned>(section.end - section.begin);

				auto chptr = section.begin + (secondary.x > len ? len : secondary.x);
				pos = static_cast<unsigned>(chptr - editor_.textbase_.getline(textline).data());
				return true;
			}

			bool _m_pos_secondary(const nana::upoint& charpos, nana::upoint& secondary_pos) const
			{
				if (charpos.y >= linemtr_.size())
					return false;

				secondary_pos.x = charpos.x;
				secondary_pos.y = 0;

				unsigned len = 0;
				auto & mtr = linemtr_[charpos.y];
				for (auto & ts : mtr.line_sections)
				{
					len = static_cast<unsigned>(ts.end - ts.begin);
					if (len >= secondary_pos.x)
						return true;

					++secondary_pos.y;
					secondary_pos.x -= static_cast<unsigned>(len);
				}
				--secondary_pos.y;
				secondary_pos.x = len;
				return true;
			}


			std::size_t _m_textline(std::size_t scrline, std::size_t & secondary) const
			{
				secondary = 0;
				std::size_t primary = 0;

				for (auto & mtr : linemtr_)
				{
					if (mtr.take_lines > scrline)
					{
						secondary = scrline;
						return primary;
					}
					else
						scrline -= mtr.take_lines;

					++primary;
				}

				return primary;
			}

			//secondary, index of line that the text was splitted into multilines.
			std::size_t _m_textline_from_screen(int y, std::size_t & secondary) const
			{
				const auto & textbase = editor_.textbase_;
				const auto & text_area = editor_.text_area_;
				auto & points = editor_.points_;

				secondary = 0;

				if (0 == textbase.lines())
					return 0;
				std::size_t screen_line;
				if (y < text_area.area.y)
				{
					screen_line = (text_area.area.y - y) / static_cast<int>(editor_.line_height());
					if (screen_line > static_cast<std::size_t>(points.offset.y))
						screen_line = 0;
					else
						screen_line = static_cast<std::size_t>(points.offset.y) - screen_line;
				}
				else
					screen_line = static_cast<std::size_t>((y - text_area.area.y) / static_cast<int>(editor_.line_height()) + points.offset.y);
				
				std::size_t primary = 0;
				for (auto & mtr : linemtr_)
				{
					if (mtr.take_lines > screen_line)
					{
						secondary = screen_line;
						return primary;
					}
					else
						screen_line -= mtr.take_lines;

					++primary;
				}
				secondary = linemtr_.back().line_sections.size() - 1;
				return linemtr_.size() - 1;
			}
		private:
			text_editor& editor_;
			std::vector<line_metrics> linemtr_;
		}; //end class behavior_linewrapped

		//class text_editor
		text_editor::text_editor(window wd, graph_reference graph)
			:	behavior_(new behavior_normal(*this)),
				window_(wd), graph_(graph), mask_char_(0)
		{
			text_area_.area = graph.size();
			text_area_.captured = false;
			text_area_.tab_space = 4;
			text_area_.scroll_pixels = 16;
			text_area_.hscroll = text_area_.vscroll = 0;
			select_.mode_selection = selection::mode_no_selected;
			select_.dragged = false;

			API::create_caret(wd, 1, line_height());
			API::background(wd, 0xFFFFFF);
			API::foreground(wd, 0x000000);
		}

		text_editor::~text_editor()
		{
			//For instance of unique_ptr pimpl idiom.
		}

		void text_editor::typeface_changed()
		{
			behavior_->pre_calc_lines(width_pixels());
		}

		bool text_editor::line_wrapped() const
		{
			return attributes_.line_wrapped;
		}

		bool text_editor::line_wrapped(bool autl)
		{
			if (autl != attributes_.line_wrapped)
			{
				attributes_.line_wrapped = autl;
				if (autl)
				{
					behavior_.reset(new behavior_linewrapped(*this));
					text_area_.vscroll = text_area_.scroll_pixels;
					text_area_.hscroll = 0;
					behavior_->pre_calc_lines(width_pixels());
				}
				else
					behavior_.reset(new behavior_normal(*this));

				points_.offset.x = 0;
				_m_offset_y(0);
				move_caret(0, 0);

				_m_scrollbar();
				render(API::is_focus_window(window_));
				return true;
			}
			return false;
		}

		void text_editor::border_renderer(std::function<void(nana::paint::graphics&, nana::color_t)> f)
		{
			text_area_.border_renderer = f;
		}

		bool text_editor::load(const nana::char_t* fs)
		{
			if (!textbase_.load(fs))
				return false;
			
			_m_reset();
			behavior_->pre_calc_lines(width_pixels());
			render(API::is_focus_window(window_));
			_m_scrollbar();
			return true;
		}

		bool text_editor::text_area(const nana::rectangle& r)
		{
			if(text_area_.area == r)
				return false;

			text_area_.area = r;
			if(attributes_.enable_counterpart)
				attributes_.counterpart.make(r.width, r.height);

			behavior_->pre_calc_lines(width_pixels());
			_m_scrollbar();
			return true;
		}

		bool text_editor::tip_string(const nana::string& str)
		{
			if(attributes_.tip_string == str)
				return false;

			attributes_.tip_string = str;
			return true;
		}

		const text_editor::attributes& text_editor::attr() const
		{
			return attributes_;
		}

		bool text_editor::multi_lines(bool ml)
		{
			if((ml == false) && attributes_.multi_lines)
			{
				//retain the first line and remove the extra lines
				if(textbase_.lines() > 1)
				{
					for(std::size_t i = textbase_.lines() - 1; i > 0; --i)
						textbase_.erase(i);
					_m_reset();
				}
			}

			if(attributes_.multi_lines != ml)
			{
				attributes_.multi_lines = ml;

				if (!ml)
					line_wrapped(false);

				_m_scrollbar();
				return true;
			}
			return false;
		}

		void text_editor::editable(bool v)
		{
			attributes_.editable = v;
		}

		void text_editor::enable_background(bool enb)
		{
			attributes_.enable_background = enb;
		}

		void text_editor::enable_background_counterpart(bool enb)
		{
			attributes_.enable_counterpart = enb;
			if(enb)
				attributes_.counterpart.make(text_area_.area.width, text_area_.area.height);
			else
				attributes_.counterpart.release();
		}

		text_editor::ext_renderer_tag& text_editor::ext_renderer() const
		{
			return ext_renderer_;
		}

		unsigned text_editor::line_height() const
		{
			return (graph_ ? (graph_.text_extent_size(STR("jH{")).height) : 0);
		}

		unsigned text_editor::screen_lines() const
		{
			if(graph_ && (text_area_.area.height > text_area_.hscroll))
			{
				auto lines = (text_area_.area.height - text_area_.hscroll) / line_height();
				return (lines ? lines : 1);
			}
			return 0;
		}

		bool text_editor::mouse_enter(bool enter)
		{
			if((false == enter) && (false == text_area_.captured))
				API::window_cursor(window_, nana::cursor::arrow);

			if(API::focus_window() != window_)
			{
				render(false);
				return true;
			}
			return false;
		}

		bool text_editor::mouse_down(bool left_button, int screen_x, int screen_y)
		{
			if(hit_text_area(screen_x, screen_y))
			{
				if(left_button)
				{
					//Set caret pos by screen point and get the caret pos.
					nana::upoint pos = mouse_caret(screen_x, screen_y);
					API::capture_window(window_, true);
					text_area_.captured = true;

					if(false == hit_select_area(pos))
					{
						if(false == select(false))
						{
							select_.a = points_.caret;	//Set begin caret
							set_end_caret();
						}
						select_.mode_selection = selection::mode_mouse_selected;
					}
					else
						select_.mode_selection = selection::mode_no_selected;
				}
				text_area_.border_renderer(graph_, _m_bgcolor());
				return true;
			}
			return false;
		}

		bool text_editor::mouse_move(bool left_button, int screen_x, int screen_y)
		{
			cursor cur = cursor::iterm;
			if((false == hit_text_area(screen_x, screen_y)) && (false == text_area_.captured))
				cur = cursor::arrow;
			
			API::window_cursor(window_, cur);

			if(left_button)
			{
				nana::upoint pos = caret();
				mouse_caret(screen_x, screen_y);

				if(select_.mode_selection != selection::mode_no_selected)
					set_end_caret();
				else if(select_.dragged == false && pos != caret())
					select_.dragged = true;

				text_area_.border_renderer(graph_, _m_bgcolor());
				return true;
			}
			return false;
		}

		bool text_editor::mouse_up(bool left_button, int screen_x, int screen_y)
		{
			bool do_draw = false;
			if(select_.mode_selection == selection::mode_mouse_selected)
			{
				select_.mode_selection = selection::mode_no_selected;
				set_end_caret();
			}
			else if(select_.mode_selection == selection::mode_no_selected)
			{
				if(select_.dragged == false || move_select() == false)
					select(false);
				do_draw = true;
			}
			select_.dragged = false;

			API::capture_window(window_, false);
			text_area_.captured = false;
			if(hit_text_area(screen_x, screen_y) == false)
				API::window_cursor(window_, nana::cursor::arrow);

			text_area_.border_renderer(graph_, _m_bgcolor());
			return do_draw;
		}

		textbase<nana::char_t> & text_editor::textbase()
		{
			return textbase_;
		}

		const textbase<nana::char_t> & text_editor::textbase() const
		{
			return textbase_;
		}

		bool text_editor::getline(std::size_t pos, nana::string& text) const
		{
			if(pos < textbase_.lines())
			{
				text = textbase_.getline(pos);
				return true;
			}
			return false;
		}

		void text_editor::setline(std::size_t n, const nana::string& text)
		{
			bool mkdraw = false;
			textbase_.replace(n, text.c_str());

			if((points_.caret.y == n) && (text.size() < points_.caret.x))
			{
				points_.caret.x = static_cast<unsigned>(text.size());
				mkdraw = behavior_->adjust_caret_into_screen();
			}

			if(!mkdraw && (static_cast<size_t>(points_.offset.y) <= n) && (n < static_cast<size_t>(points_.offset.y + screen_lines())))
				mkdraw = true;

			if(mkdraw)
				render(API::focus_window() == window_);
		}

		void text_editor::text(const nana::string& str)
		{
			textbase_.erase_all();
			_m_reset();
			put(str);
		}

		nana::string text_editor::text() const
		{
			nana::string str;
			std::size_t lines = textbase_.lines();
			if(lines > 0)
			{
				str += textbase_.getline(0);
				for(std::size_t i = 1; i < lines; ++i)
				{
					str += STR("\n\r");
					str += textbase_.getline(i);
				}
			}
			return str;
		}

		//move_caret
		//Set caret position through text coordinate
		void text_editor::move_caret(size_type x, size_type y)
		{
			if(API::is_focus_window(window_))
			{
				const unsigned line_pixels = line_height();
				auto pos = this->behavior_->caret_to_screen(x, y);
				const int end_y = pos.y + static_cast<int>(line_pixels);

				bool visible = true;
				if (pos.x < static_cast<int>(text_area_.area.x) || _m_endx() < pos.x)
					visible = false;
				else if (end_y <= 0 || pos.y >= _m_endy())
					visible = false;
				else if (end_y > _m_endy())
					API::caret_size(window_, nana::size(1, line_pixels - (end_y - _m_endy())));
				else if (API::caret_size(window_).height != line_pixels)
					reset_caret_height();
				
				if(visible != API::caret_visible(window_))
					API::caret_visible(window_, visible);

				if(visible)
					API::caret_pos(window_, pos.x, pos.y);
			}
		}

		void text_editor::move_caret_end()
		{
			points_.caret.y = static_cast<unsigned>(textbase_.lines());
			if(points_.caret.y) --points_.caret.y;
			points_.caret.x = static_cast<unsigned>(textbase_.getline(points_.caret.y).size());
		}

		void text_editor::reset_caret_height() const
		{
			API::caret_size(window_, nana::size(1, line_height()));
		}

		void text_editor::reset_caret()
		{
			move_caret(points_.caret.x, points_.caret.y);
		}

		void text_editor::show_caret(bool isshow)
		{
			if(isshow == false || API::is_focus_window(window_))
				API::caret_visible(window_, isshow);
		}

		bool text_editor::selected() const
		{
			return (select_.a != select_.b);
		}

		void text_editor::set_end_caret()
		{
			bool new_sel_end = (select_.b != points_.caret);
			select_.b = points_.caret;
			points_.xpos = points_.caret.x;

			if (new_sel_end || behavior_->adjust_caret_into_screen())
				render(true);
		}

		bool text_editor::select(bool yes)
		{
			if(yes)
			{
				select_.a.x = select_.a.y = 0;
				select_.b.y = static_cast<unsigned>(textbase_.lines());
				if(select_.b.y) --select_.b.y;
				select_.b.x = static_cast<unsigned>(textbase_.getline(select_.b.y).size());
				select_.mode_selection = selection::mode_method_selected;
				return true;
			}

			select_.mode_selection = selection::mode_no_selected;
			if(_m_cancel_select(0))
			{
				render(true);
				return true;
			}
			return false;
		}

		bool text_editor::hit_text_area(int x, int y) const
		{
			return ((text_area_.area.x <= x && x < _m_endx()) && (text_area_.area.y <= y && y < _m_endy()));
		}

		bool text_editor::hit_select_area(nana::upoint pos) const
		{
			nana::upoint a, b;
			_m_get_sort_select_points(a, b);
			if(a != b)
			{
				if((pos.y > a.y || (pos.y == a.y && pos.x >= a.x)) && ((pos.y < b.y) || (pos.y == b.y && pos.x < b.x)))
					return true;
			}
			return false;
		}

		bool text_editor::move_select()
		{
			nana::upoint a, b;
			_m_get_sort_select_points(a, b);

			if(hit_select_area(points_.caret) || (select_.b == points_.caret))
			{
				points_.caret = select_.b;

				if (behavior_->adjust_caret_into_screen())
					render(true);

				reset_caret();
				return true;
			}

			nana::upoint caret = points_.caret;

			nana::string text;
			if(_m_make_select_string(text))
			{
				if(caret.y < a.y || (caret.y == a.y && caret.x < a.x))
				{//forward
					_m_erase_select();
					_m_put(text);

					select_.a = caret;
					select_.b.y = b.y + (caret.y - a.y);
				}
				else if(b.y < caret.y || (caret.y == b.y && b.x < caret.x))
				{
					_m_put(text);
					_m_erase_select();

					select_.b.y = caret.y;
					select_.a.y = caret.y - (b.y - a.y);
					select_.a.x = caret.x - (caret.y == b.y ? (b.x - a.x) : 0);
				}
				select_.b.x = b.x + (a.y == b.y ? (select_.a.x - a.x) : 0);

				points_.caret = select_.a;
				reset_caret();

				behavior_->adjust_caret_into_screen();
				render(true);
				return true;
			}
			return false;
		}

		bool text_editor::mask(nana::char_t ch)
		{
			if(mask_char_ != ch)
			{
				mask_char_ = ch;
				return true;
			}
			return false;
		}

		unsigned text_editor::width_pixels() const
		{
			if (attributes_.line_wrapped)
				return (text_area_.area.width > text_area_.vscroll ? text_area_.area.width - text_area_.vscroll : 0);

			auto caret_px = API::caret_size(window_).width;
			return (text_area_.area.width > caret_px ? text_area_.area.width - caret_px : 0);
		}

		void text_editor::draw_scroll_rectangle()
		{
			if(text_area_.vscroll && text_area_.hscroll)
			{
				graph_.rectangle( text_area_.area.x + static_cast<int>(text_area_.area.width - text_area_.vscroll),
									text_area_.area.y + static_cast<int>(text_area_.area.height - text_area_.hscroll),
									text_area_.vscroll, text_area_.hscroll, nana::color::button_face, true);
			}
		}

		void text_editor::render(bool has_focus)
		{
			const nana::color_t bgcolor = _m_bgcolor();

			nana::color_t fgcolor = API::foreground(window_);
			if (!API::window_enabled(window_))
				fgcolor = nana::paint::graphics::mix(bgcolor, fgcolor, 0.5);

			//Draw background
			if(attributes_.enable_background)
				graph_.rectangle(text_area_.area, bgcolor, true);

			if(ext_renderer_.background)
				ext_renderer_.background(graph_, text_area_.area, bgcolor);

			if(attributes_.counterpart && text_area_.area.width && text_area_.area.height)
				attributes_.counterpart.bitblt(nana::rectangle(0, 0, text_area_.area.width, text_area_.area.height), graph_, nana::point(text_area_.area.x, text_area_.area.y));

			if((false == textbase_.empty()) || has_focus)
				behavior_->render(fgcolor);
			else
				_m_draw_tip_string();

			draw_scroll_rectangle();
			text_area_.border_renderer(graph_, bgcolor);
		}
	//public:
		void text_editor::put(const nana::string& text)
		{
			//Do not forget to assign the _m_erase_select() to caret
			//because _m_put() will insert the text at the position where the caret is.
			points_.caret = _m_erase_select();
			points_.caret = _m_put(text);

			if(graph_)
			{
				behavior_->adjust_caret_into_screen();
				reset_caret();
				render(API::is_focus_window(window_));
				_m_scrollbar();

				points_.xpos = points_.caret.x;
			}
		}

		void text_editor::put(nana::char_t c)
		{
			bool refresh = (select_.a != select_.b);
			if(refresh)
				points_.caret = _m_erase_select();

			auto secondary_before = behavior_->take_lines(points_.caret.y);
			textbase_.insert(points_.caret.y, points_.caret.x, c);
			behavior_->pre_calc_line(points_.caret.y, width_pixels());
			points_.caret.x ++;

			if(refresh || _m_draw(c, secondary_before))
				render(true);
			else
				draw_scroll_rectangle();

			_m_scrollbar();

			points_.xpos = points_.caret.x;
		}

		void text_editor::copy() const
		{
			nana::string str;
			if(_m_make_select_string(str))
			{
				nana::system::dataexch de;
				de.set(str);
			}
		}

		void text_editor::paste()
		{
			points_.caret = _m_erase_select();

			nana::system::dataexch de;
			nana::string text;
			de.get(text);
			put(text);
		}

		void text_editor::enter()
		{
			if(false == attributes_.multi_lines)
				return;

			bool need_refresh = (select_.a != select_.b);

			if(need_refresh)
				points_.caret = _m_erase_select();

			const string_type& lnstr = textbase_.getline(points_.caret.y);
			++points_.caret.y;

			if(lnstr.size() > points_.caret.x)
			{
				textbase_.insertln(points_.caret.y, lnstr.c_str() + points_.caret.x);
				textbase_.erase(points_.caret.y - 1, points_.caret.x, lnstr.size() - points_.caret.x);
			}
			else
			{
				if(textbase_.lines() == 0) textbase_.insertln(0, STR(""));
				textbase_.insertln(points_.caret.y, STR(""));
			}

			const auto width_px = width_pixels();
			behavior_->add_lines(points_.caret.y - 1, 1);
			behavior_->pre_calc_line(points_.caret.y, width_px);
			behavior_->pre_calc_line(points_.caret.y - 1, width_px);

			points_.caret.x = 0;

			if(points_.offset.x || (points_.caret.y < textbase_.lines()) || textbase_.getline(points_.caret.y).size())
			{
				points_.offset.x = 0;
				need_refresh = true;
			}

			if (behavior_->adjust_caret_into_screen() || need_refresh)
				render(true);

			_m_scrollbar();
		}

		void text_editor::del()
		{
			bool has_erase = true;

			if(select_.a == select_.b)
			{
				if(textbase_.getline(points_.caret.y).size() > points_.caret.x)
				{
					points_.caret.x++;
				}
				else if(textbase_.lines() && (points_.caret.y < textbase_.lines() - 1))
				{	//Move to next line
					points_.caret.x = 0;
					++ points_.caret.y;
				}
				else
					has_erase = false;	//No characters behind the caret
			}

			if(has_erase)	backspace();

			_m_scrollbar();
			points_.xpos = points_.caret.x;
		}

		void text_editor::backspace()
		{
			bool has_to_redraw = true;
			if(select_.a == select_.b)
			{
				if(points_.caret.x)
				{
					unsigned erase_number = 1;
					--points_.caret.x;
#ifndef NANA_UNICODE
					const string_type& lnstr = textbase_.getline(points_.caret.y);
					if(is_incomplete(lnstr, points_.caret.x) && (points_.caret.x))
					{
						textbase_.erase(points_.caret.y, points_.caret.x, 1);
						--points_.caret.x;
						erase_number = 2;
					}
#endif
					auto secondary = behavior_->take_lines(points_.caret.y);
					textbase_.erase(points_.caret.y, points_.caret.x, erase_number);
					behavior_->pre_calc_line(points_.caret.y, width_pixels());
					if(_m_move_offset_x_while_over_border(-2) == false)
					{
						behavior_->update_line(points_.caret.y, secondary);
						draw_scroll_rectangle();
						has_to_redraw = false;
					}
				}
				else if(points_.caret.y)
				{
					points_.caret.x = static_cast<unsigned>(textbase_.getline(-- points_.caret.y).size());
					textbase_.merge(points_.caret.y);
					behavior_->merge_lines(points_.caret.y, points_.caret.y + 1);
				}
			}
			else
				points_.caret = _m_erase_select();

			if(has_to_redraw)
			{
				behavior_->pre_calc_lines(width_pixels());
				behavior_->adjust_caret_into_screen();
				render(true);
			}
			_m_scrollbar();
		}

		bool text_editor::move(nana::char_t key)
		{
			switch(key)
			{
			case keyboard::os_arrow_left:	move_left();	break;
			case keyboard::os_arrow_right:	move_right();	break;
			case keyboard::os_arrow_up:		move_up();		break;
			case keyboard::os_arrow_down:	move_down();	break;
			case keyboard::os_del:
				if (this->attr().editable)
					del();
				break;
			default:
				return false;
			}
			return true;
		}

		void text_editor::move_up()
		{
			const bool redraw_required = _m_cancel_select(0);
			if (behavior_->move_caret_ns(true) || redraw_required)
				render(true);
			_m_scrollbar();
		}

		void text_editor::move_down()
		{
			const bool redraw_required = _m_cancel_select(0);
			if(behavior_->move_caret_ns(false) || redraw_required)
				render(true);
			_m_scrollbar();
		}

		void text_editor::move_left()
		{
			if(_m_cancel_select(1) == false)
			{
				if(points_.caret.x)
				{
					--points_.caret.x;
#ifndef NANA_UNICODE
					if(is_incomplete(textbase_.getline(points_.caret.y), points_.caret.x))
						--points_.caret.x;
#endif
					bool adjust_y = false;
					if (attributes_.line_wrapped)
						adjust_y = behavior_->adjust_caret_into_screen();

					bool adjust_x = _m_move_offset_x_while_over_border(-2);

					if (adjust_x || adjust_y)
						render(true);
				}
				else if(points_.caret.y)
				{	//Move to previous line
					points_.caret.x = static_cast<unsigned>(textbase_.getline(-- points_.caret.y).size());
					if (behavior_->adjust_caret_into_screen())
						render(true);
				}
			}
			else
			{
				behavior_->adjust_caret_into_screen();
				render(true);
			}

			_m_scrollbar();
			points_.xpos = points_.caret.x;
		}

		void text_editor::move_right()
		{
			if(_m_cancel_select(2) == false)
			{
				nana::string lnstr = textbase_.getline(points_.caret.y);
				if(lnstr.size() > points_.caret.x)
				{
					++points_.caret.x;
#ifndef NANA_UNICODE
					if(is_incomplete(lnstr, points_.caret.x))
						++points_.caret.x;
#endif
					bool adjust_y = false;
					if (attributes_.line_wrapped)
						adjust_y = behavior_->adjust_caret_into_screen();

					bool adjust_x = _m_move_offset_x_while_over_border(2);

					if (adjust_x || adjust_y)
						render(true);
				}
				else if(textbase_.lines() && (points_.caret.y < textbase_.lines() - 1))
				{	//Move to next line
					points_.caret.x = 0;
					++ points_.caret.y;

					if (behavior_->adjust_caret_into_screen())
						render(true);
				}
			}
			else
			{
				if (behavior_->adjust_caret_into_screen())
					render(true);
			}

			_m_scrollbar();
			points_.xpos = points_.caret.x;
		}

		nana::upoint text_editor::mouse_caret(int screen_x, int screen_y)
		{
			points_.caret = behavior_->screen_to_caret(screen_x, screen_y);

			if (behavior_->adjust_caret_into_screen())
				render(true);

			move_caret(points_.caret.x, points_.caret.y);
			return points_.caret;
		}

		nana::upoint text_editor::caret() const
		{
			return points_.caret;
		}

		bool text_editor::scroll(bool upwards, bool vertical)
		{
			if(vertical && attributes_.vscroll)
			{
				attributes_.vscroll->make_step(!upwards);
				if(_m_scroll_text(true))
				{
					render(true);
					return true;
				}
			}
			return false;
		}

	//private:
		nana::color_t text_editor::_m_bgcolor() const
		{
			return (!API::window_enabled(window_) ? 0xE0E0E0 : API::background(window_));
		}

		bool text_editor::_m_scroll_text(bool vertical)
		{
			if(attributes_.vscroll && vertical)
			{
				if(static_cast<int>(attributes_.vscroll->value()) != points_.offset.y)
				{
					_m_offset_y(static_cast<int>(attributes_.vscroll->value()));
					return true;
				}
			}
			else if(attributes_.hscroll && (vertical == false))
			{
				if(static_cast<int>(attributes_.hscroll->value()) != points_.offset.x)
				{
					points_.offset.x = static_cast<int>(attributes_.hscroll->value());
					return true;
				}
			}
			return false;
		}

		void text_editor::_m_on_scroll(const arg_mouse& arg)
		{
			if((arg.evt_code == event_code::mouse_move) && (arg.left_button == false))
				return;

			bool vertical;
			if(attributes_.vscroll && (attributes_.vscroll->handle() == arg.window_handle))
				vertical = true;
			else if(attributes_.hscroll && (attributes_.hscroll->handle() == arg.window_handle))
				vertical = false;
			else
				return;

			if(_m_scroll_text(vertical))
			{
				render(true);
				reset_caret();
				API::update_window(window_);
			}
		}

		void text_editor::_m_scrollbar()
		{
			_m_get_scrollbar_size();

			nana::size tx_area = _m_text_area();
			if (text_area_.vscroll)
			{
				auto wdptr = attributes_.vscroll.get();
				int x = text_area_.area.x + static_cast<int>(tx_area.width);
				if (nullptr == wdptr)
				{
					std::unique_ptr<nana::scroll<true>> scptr(new nana::scroll<true>);
					wdptr = scptr.get();
					wdptr->create(window_, nana::rectangle(x, text_area_.area.y, text_area_.vscroll, tx_area.height));

					auto fn = [this](const arg_mouse& arg)
					{
						_m_on_scroll(arg);
					};
					wdptr->events().mouse_down.connect(fn);
					wdptr->events().mouse_move.connect(fn);
					wdptr->events().mouse_wheel.connect([this](const arg_wheel& arg)
					{
						_m_on_scroll(arg);
					});

					attributes_.vscroll.swap(scptr);
					API::take_active(wdptr->handle(), false, window_);
				}

				if (behavior_->take_lines() != wdptr->amount())
					wdptr->amount(static_cast<int>(behavior_->take_lines()));

				if (screen_lines() != wdptr->range())
					wdptr->range(screen_lines());

				if (points_.offset.y != static_cast<int>(wdptr->value()))
					wdptr->value(points_.offset.y);

				wdptr->move(x, text_area_.area.y, text_area_.vscroll, tx_area.height);
			}
			else
				attributes_.vscroll.reset();

			//HScroll
			if(text_area_.hscroll)
			{
				auto wdptr = attributes_.hscroll.get();
				int y = text_area_.area.y + static_cast<int>(tx_area.height);
				if(nullptr == wdptr)
				{
					std::unique_ptr<nana::scroll<false>> scptr(new nana::scroll<false>);
					wdptr = scptr.get();
					wdptr->create(window_, nana::rectangle(text_area_.area.x, y, tx_area.width, text_area_.hscroll));

					auto fn = [this](const arg_mouse& arg)
					{
						_m_on_scroll(arg);
					};
					wdptr->events().mouse_down.connect(fn);
					wdptr->events().mouse_move.connect(fn);
					wdptr->events().mouse_wheel.connect(fn);
					wdptr->step(20);
					attributes_.hscroll.swap(scptr);
					API::take_active(wdptr->handle(), false, window_);
				}
				auto maxline = textbase_.max_line();
				nana::size text_size = _m_text_extent_size(textbase_.getline(maxline.first).c_str(), maxline.second);

				text_size.width += 1;
				if(text_size.width > wdptr->amount())
					wdptr->amount(text_size.width);

				if(tx_area.width != wdptr->range())	wdptr->range(tx_area.width);
				if(points_.offset.x != static_cast<int>(wdptr->value()))
					wdptr->value(points_.offset.x);

				wdptr->move(text_area_.area.x, y, tx_area.width, text_area_.hscroll);
			}
			else
				attributes_.hscroll.reset();
		}

		nana::size text_editor::_m_text_area() const
		{
			return nana::size((text_area_.area.width > text_area_.vscroll ? text_area_.area.width - text_area_.vscroll : 0),
				(text_area_.area.height > text_area_.hscroll ? text_area_.area.height - text_area_.hscroll : 0));
		}

		void text_editor::_m_get_scrollbar_size()
		{
			text_area_.hscroll = 0;

			if (attributes_.line_wrapped)
			{
				text_area_.vscroll = text_area_.scroll_pixels;
				return;
			}

			//Only the textbox is multi_lines, it enables the scrollbars
			if(attributes_.multi_lines)
			{
				text_area_.vscroll = (textbase_.lines() > screen_lines() ? text_area_.scroll_pixels : 0);

				std::pair<size_t, size_t> max_line = textbase_.max_line();
				if(max_line.second)
				{
					if(points_.offset.x || _m_text_extent_size(textbase_.getline(max_line.first).c_str(), max_line.second).width > _m_text_area().width)
					{
						text_area_.hscroll = text_area_.scroll_pixels;
						if((text_area_.vscroll == 0) && (textbase_.lines() > screen_lines()))
							text_area_.vscroll = text_area_.scroll_pixels;
					}
				}
			}
			else
				text_area_.vscroll = 0;
		}

		void text_editor::_m_reset()
		{
			points_.caret.x = points_.caret.y = 0;
			points_.offset.x = 0;
			_m_offset_y(0);
			select_.a = select_.b;
		}

		nana::upoint text_editor::_m_put(nana::string text)
		{
			std::size_t lines = _m_make_simple_nl(text);

			nana::upoint caret = points_.caret;
			if(lines > 1)
			{
				nana::string orig_str = textbase_.getline(caret.y);
				unsigned orig_x = caret.x;

				nana::string::size_type beg = 0, end = text.find('\n');
				if(attributes_.multi_lines)
				{
					if(orig_str.size() == orig_x)
						textbase_.insert(caret.y, caret.x, text.substr(beg, end - beg).c_str());
					else
						textbase_.replace(caret.y, (orig_str.substr(0, orig_x) + text.substr(beg, end - beg)).c_str());

					std::size_t n = 2;
					++caret.y;
					beg = end + 1;

					end = text.find('\n', beg);

					while(end != nana::string::npos)
					{
						if(n != lines)
							textbase_.insertln(caret.y, text.substr(beg, end - beg).c_str());

						beg = end + 1;
						caret.y++;
						++n;

						end = text.find('\n', beg);
					}

					textbase_.insertln(caret.y, (text.substr(beg) + orig_str.substr(orig_x)).c_str());
					caret.x = static_cast<unsigned>(text.size() - beg);
				}
				else
				{
					nana::string newstr = text.substr(beg, end);
					textbase_.insert(caret.y, caret.x, newstr.c_str());
					caret.x += static_cast<unsigned>(newstr.size());
				}

				const auto width_px = width_pixels();
				behavior_->add_lines(points_.caret.y, lines - 1);
				const auto endline = points_.caret.y + lines;
				for (auto i = points_.caret.y; i < endline; ++i)
					behavior_->pre_calc_line(i, width_px);
			}
			else
			{
				textbase_.insert(caret.y, caret.x, text.c_str());
				caret.x += static_cast<unsigned>(text.size());
				behavior_->pre_calc_line(caret.y, width_pixels());
			}
			return caret;
		}

		nana::upoint text_editor::_m_erase_select()
		{
			nana::upoint a, b;
			_m_get_sort_select_points(a, b);
			if(a != b)
			{
				if(a.y != b.y)
				{
					textbase_.erase(a.y, a.x, nana::string::npos);

					for(unsigned ln = a.y + 1; ln < b.y; ++ln)
						textbase_.erase(a.y + 1);

					textbase_.erase(a.y + 1, 0, b.x);
					textbase_.merge(a.y);

					behavior_->merge_lines(a.y, b.y);
				}
				else
				{
					textbase_.erase(a.y, a.x, b.x - a.x);
					behavior_->pre_calc_line(a.y, width_pixels());
				}

				select_.a = select_.b;
				return a;
			}

			return points_.caret;
		}

		bool text_editor::_m_make_select_string(nana::string& text) const
		{
			nana::upoint a, b;
			_m_get_sort_select_points(a, b);
			if(a != b)
			{
				if(a.y != b.y)
				{
					text = textbase_.getline(a.y).substr(a.x);
					text += STR("\r\n");
					for(unsigned i = a.y + 1; i < b.y; ++i)
					{
						text += textbase_.getline(i).substr(0);
						text += STR("\r\n");
					}
					text += textbase_.getline(b.y).substr(0, b.x);
				}
				else
					text = textbase_.getline(a.y).substr(a.x, b.x - a.x);

				return true;
			}
			return false;
		}

		//_m_make_simple_nl
		//@brief: transform a string if it contains "0xD\0xA" or "0xA\0xD"
		std::size_t text_editor::_m_make_simple_nl(nana::string& text)
		{
			std::size_t lines = 1;
			nana::string::size_type beg = 0;

			while(true)
			{
				auto nl = text.find('\n', beg);
				if(nana::string::npos == nl) break;

				if(nl && (text[nl - 1] == 0xD))
					text.erase(--nl, 1);
				else if(nl < text.size() - 1 && text[nl + 1] == 0xD)
					text.erase(nl + 1, 1);

				beg = nl + 1;
				++lines;
			}

			auto pos = text.find_last_not_of(nana::char_t(0));
			if(pos != text.npos)
				text.erase(pos + 1);
			return lines;
		}

		bool text_editor::_m_cancel_select(int align)
		{
			if(select_.a != select_.b)
			{
				nana::upoint a, b;
				_m_get_sort_select_points(a, b);

				switch(align)
				{
				case 1:
					points_.caret = a;
					_m_move_offset_x_while_over_border(-2);
					break;
				case 2:
					points_.caret = b;
					_m_move_offset_x_while_over_border(2);
					break;
				}
				select_.a = select_.b = points_.caret;
				reset_caret();
				return true;
			}
			return false;
		}

		unsigned text_editor::_m_tabs_pixels(size_type tabs) const
		{
			if(0 == tabs) return 0;

			nana::char_t ws[2] = {};
			ws[0] = mask_char_ ? mask_char_ : ' ';
			return static_cast<unsigned>(tabs * graph_.text_extent_size(ws).width * text_area_.tab_space);
		}

		nana::size text_editor::_m_text_extent_size(const char_type* str) const
		{
			return _m_text_extent_size(str, nana::strlen(str));
		}

		nana::size text_editor::_m_text_extent_size(const char_type* str, size_type n) const
		{
			if(graph_)
			{
				if(mask_char_)
				{
					nana::string maskstr;
					maskstr.append(n, mask_char_);
					return graph_.text_extent_size(maskstr);
				}
				else
					return graph_.text_extent_size(str, static_cast<unsigned>(n));
			}
			return nana::size();
		}

		//_m_move_offset_x_while_over_border
		//@brief: Move the view window
		bool text_editor::_m_move_offset_x_while_over_border(int many)
		{
			//x never beyonds border in line-wrapped mode.
			if (attributes_.line_wrapped)
				return false;

			const string_type& lnstr = textbase_.getline(points_.caret.y);
			unsigned width = _m_text_extent_size(lnstr.c_str(), points_.caret.x).width;
			if(many < 0)
			{
				many = -many;
				if(points_.offset.x && (points_.offset.x >= static_cast<int>(width)))
				{	//Out of screen text area
					if(points_.caret.x > static_cast<unsigned>(many))
						points_.offset.x = static_cast<int>(width - _m_text_extent_size(lnstr.c_str() + points_.caret.x - many, many).width);
					else
						points_.offset.x = 0;
					return true;
				}
			}
			else if(many)
			{
				width += text_area_.area.x;
				if(static_cast<int>(width) - points_.offset.x >= _m_endx())
				{	//Out of screen text area
					points_.offset.x = width - _m_endx() + 1;
					string_type::size_type rest_size = lnstr.size() - points_.caret.x;
					points_.offset.x += static_cast<int>(_m_text_extent_size(lnstr.c_str() + points_.caret.x, (rest_size >= static_cast<unsigned>(many) ? static_cast<unsigned>(many) : rest_size)).width);
					return true;
				}
			}
			return false;
		}

		int text_editor::_m_text_top_base() const
		{
			if(false == attributes_.multi_lines)
			{
				unsigned px = line_height();
				if(text_area_.area.height > px)
					return text_area_.area.y + static_cast<int>((text_area_.area.height - px) >> 1);
			}
			return text_area_.area.y;
		}

		//_m_endx
		//@brief: Get the right point of text area
		int text_editor::_m_endx() const
		{
			return static_cast<int>(text_area_.area.x + text_area_.area.width - text_area_.vscroll);
		}

		//_m_endy
		//@brief: Get the bottom point of text area
		int text_editor::_m_endy() const
		{
			return static_cast<int>(text_area_.area.y + text_area_.area.height - text_area_.hscroll);
		}

		void text_editor::_m_draw_tip_string() const
		{
			graph_.string(text_area_.area.x - points_.offset.x, text_area_.area.y, 0x787878, attributes_.tip_string);
		}

		void text_editor::_m_draw_string(int top, nana::color_t color, const nana::upoint& str_pos, const nana::string& linestr, bool if_mask) const
		{
			int x = text_area_.area.x - points_.offset.x;
			int xend = text_area_.area.x + static_cast<int>(text_area_.area.width);

			if (if_mask && mask_char_)
			{
				nana::string maskstr;
				maskstr.append(linestr.size(), mask_char_);
				graph_.string(x, top, color, maskstr);
				return;
			}

			unicode_bidi bidi;
			std::vector<unicode_bidi::entity> reordered;
			bidi.linestr(linestr.c_str(), linestr.size(), reordered);

			unsigned whitespace_w = graph_.text_extent_size(STR(" ")).width;

			const unsigned line_h_pixels = line_height();

			//The line of text is in the range of selection
			nana::upoint a, b;
			_m_get_sort_select_points(a, b);

			//The text is not selected or the whole line text is selected
			if ((select_.a == select_.b) || (select_.a.y != str_pos.y && select_.b.y != str_pos.y))
			{
				bool selected = (a.y < static_cast<unsigned>(str_pos.y) && static_cast<unsigned>(str_pos.y) < b.y);
				for (auto & ent : reordered)
				{
					std::size_t len = ent.end - ent.begin;
					unsigned str_w = graph_.text_extent_size(ent.begin, len).width;

					if ((x + static_cast<int>(str_w) > text_area_.area.x) && (x < xend))
					{
						if (selected)
						{
							color = 0xFFFFFF;
							graph_.rectangle(x, top, str_w, line_h_pixels, 0x3399FF, true);
						}

						graph_.string(x, top, color, ent.begin, len);
					}
					x += static_cast<int>(str_w);
				}
				if (selected)
					graph_.rectangle(x, top, whitespace_w, line_h_pixels, 0x3399FF, true);
			}
			else
			{
				const nana::char_t * strbeg = linestr.c_str();
				if (a.y == b.y)
				{
					for (auto & ent : reordered)
					{
						std::size_t len = ent.end - ent.begin;
						unsigned str_w = graph_.text_extent_size(ent.begin, len).width;
						if ((x + static_cast<int>(str_w) > text_area_.area.x) && (x < xend))
						{
							std::size_t pos = ent.begin - strbeg + str_pos.x;

							if (pos + len <= a.x || pos >= b.x)
							{
								//NOT selected
								graph_.string(x, top, color, ent.begin, len);
							}
							else if (a.x <= pos && pos + len <= b.x)
							{
								//Whole selected
								graph_.rectangle(x, top, str_w, line_h_pixels, 0x3399FF, true);
								graph_.string(x, top, 0xFFFFFF, ent.begin, len);
							}
							else if (pos <= a.x && a.x < pos + len)
							{	//Partial selected
								int endpos = static_cast<int>(b.x < pos + len ? b.x : pos + len);
								unsigned * pxbuf = new unsigned[len];
								if (graph_.glyph_pixels(ent.begin, len, pxbuf))
								{
									unsigned head_w = 0;
									for (std::size_t u = 0; u < a.x - pos; ++u)
										head_w += pxbuf[u];

									unsigned sel_w = 0;
									for (std::size_t u = a.x - pos; u < endpos - pos; ++u)
										sel_w += pxbuf[u];

									if (_m_is_right_text(ent))
									{	//RTL
										graph_.string(x, top, color, ent.begin, len);
										nana::paint::graphics graph(str_w, line_h_pixels);
										graph.typeface(graph_.typeface());
										graph.rectangle(0x3399FF, true);
										graph.string(0, 0, 0xFFFFFF, ent.begin, len);

										int sel_xpos = static_cast<int>(str_w - head_w - sel_w);
										graph_.bitblt(nana::rectangle(x + sel_xpos, top, sel_w, line_h_pixels), graph, nana::point(sel_xpos, 0));
									}
									else
									{	//LTR
										graph_.string(x, top, color, ent.begin, a.x - pos);

										graph_.rectangle(x + head_w, top, sel_w, line_h_pixels, 0x3399FF, true);
										graph_.string(x + head_w, top, 0xFFFFFF, ent.begin + (a.x - pos), endpos - a.x);

										if (static_cast<size_t>(endpos) < pos + len)
											graph_.string(x + static_cast<int>(head_w + sel_w), top, color, ent.begin + (endpos - pos), pos + len - endpos);
									}
								}
								delete[] pxbuf;
							}
							else if (pos <= b.x && b.x < pos + len)
							{	//Partial selected
								int endpos = b.x;
								unsigned sel_w = graph_.glyph_extent_size(ent.begin, len, 0, endpos - pos).width;
								if (_m_is_right_text(ent))
								{	//RTL
									graph_.string(x, top, color, ent.begin, len);
									nana::paint::graphics graph(str_w, line_h_pixels);
									graph.typeface(graph_.typeface());
									graph.rectangle(0x3399FF, true);
									graph.string(0, 0, 0xFFFFFF, ent.begin, len);
									graph_.bitblt(nana::rectangle(x + (str_w - sel_w), top, sel_w, line_h_pixels), graph, nana::point(str_w - sel_w, 0));
								}
								else
								{	//LTR
									graph_.rectangle(x, top, sel_w, line_h_pixels, 0x3399FF, true);
									graph_.string(x, top, 0xFFFFFF, ent.begin, endpos - pos);
									graph_.string(x + sel_w, top, color, ent.begin + (endpos - pos), pos + len - endpos);
								}
							}
						}
						x += static_cast<int>(str_w);
					}
				}
				else if (a.y == str_pos.y)
				{
					for (auto & ent : reordered)
					{
						std::size_t len = ent.end - ent.begin;
						unsigned str_w = graph_.text_extent_size(ent.begin, len).width;
						if ((x + static_cast<int>(str_w) > text_area_.area.x) && (x < xend))
						{
							std::size_t pos = ent.begin - strbeg + str_pos.x;
							if (pos + len <= a.x)
							{
								//Not selected
								graph_.string(x, top, color, ent.begin, len);
							}
							else if (a.x < pos)
							{
								//Whole selected
								graph_.rectangle(x, top, str_w, line_h_pixels, 0x3399FF, true);
								graph_.string(x, top, 0xFFFFFF, ent.begin, len);
							}
							else
							{
								unsigned head_w = graph_.glyph_extent_size(ent.begin, len, 0, a.x - pos).width;
								if (_m_is_right_text(ent))
								{	//RTL
									graph_.string(x, top, color, ent.begin, len);
									nana::paint::graphics graph(str_w, line_h_pixels);
									graph.typeface(graph_.typeface());
									graph.rectangle(0x3399FF, true);
									graph.string(0, 0, 0xFFFFFF, ent.begin, len);
									graph_.bitblt(nana::rectangle(x, top, str_w - head_w, line_h_pixels), graph);
								}
								else
								{	//LTR
									graph_.string(x, top, color, ent.begin, a.x - pos);
									graph_.rectangle(x + head_w, top, str_w - head_w, line_h_pixels, 0x3399FF, true);
									graph_.string(x + head_w, top, 0xFFFFFF, ent.begin + a.x - pos, len - (a.x - pos));
								}
							}
						}

						x += static_cast<int>(str_w);
					}
					if (a.y <= static_cast<unsigned>(str_pos.y) && static_cast<unsigned>(str_pos.y) < b.y)
						graph_.rectangle(x, top, whitespace_w, line_h_pixels, 0x3399FF, true);
				}
				else if (b.y == str_pos.y)
				{
					for (auto & ent : reordered)
					{
						std::size_t len = ent.end - ent.begin;
						unsigned str_w = graph_.text_extent_size(ent.begin, len).width;
						if ((x + static_cast<int>(str_w) > text_area_.area.x) && (x < xend))
						{
							std::size_t pos = ent.begin - strbeg + str_pos.x;

							if (pos + len <= b.x)
							{
								graph_.rectangle(x, top, str_w, line_h_pixels, 0x3399FF, true);
								graph_.string(x, top, 0xFFFFFF, ent.begin, len);
							}
							else if (pos <= b.x && b.x < pos + len)
							{
								unsigned sel_w = graph_.glyph_extent_size(ent.begin, len, 0, b.x - pos).width;
								if (_m_is_right_text(ent))
								{
									nana::paint::graphics graph(str_w, line_h_pixels);
									graph.typeface(graph_.typeface());
									graph.rectangle(0x3399FF, true);
									graph.string(0, 0, 0xFFFFFF, ent.begin, len);
									graph_.string(x, top, color, ent.begin, len);

									graph_.bitblt(nana::rectangle(x + (str_w - sel_w), top, sel_w, line_h_pixels), graph, nana::point(str_w - sel_w, 0));
								}
								else
								{
									graph_.rectangle(x, top, sel_w, line_h_pixels, 0x3399FF, true);
									graph_.string(x, top, 0xFFFFFF, ent.begin, b.x - pos);
									graph_.string(x + sel_w, top, color, ent.begin + b.x - pos, len - (b.x - pos));
								}
							}
							else
								graph_.string(x, top, color, ent.begin, len);
						}
						x += static_cast<int>(str_w);
					}
				}
			}
		}

		//_m_draw
		//@brief: Draw a character at a position specified by caret pos.
		//@return: true if beyond the border
		bool text_editor::_m_draw(nana::char_t c, std::size_t secondary_before)
		{
			if (false == behavior_->adjust_caret_into_screen())
			{
				if (behavior_->caret_to_screen(points_.caret.x, points_.caret.y).x < _m_endx())
				{
					behavior_->update_line(points_.caret.y, secondary_before);
					return false;
				}
			}
			return true;
		}

		void text_editor::_m_get_sort_select_points(nana::upoint& a, nana::upoint& b) const
		{
			if((select_.a.y > select_.b.y) || ((select_.a.y == select_.b.y) && (select_.a.x > select_.b.x)))
			{
				a = select_.b;
				b = select_.a;
			}
			else
			{
				a = select_.a;
				b = select_.b;
			}
		}

		void text_editor::_m_offset_y(int y)
		{
			points_.offset.y = y;
		}

		unsigned text_editor::_m_char_by_pixels(const nana::char_t* str, std::size_t len, unsigned * pxbuf, int str_px, int pixels, bool is_rtl)
		{
			unsigned pos = 0;	//Result
			if (graph_.glyph_pixels(str, len, pxbuf))
			{
				if (is_rtl)
				{	//RTL
					for (std::size_t u = 0; u < len; ++u)
					{
						int chbeg = (str_px - pxbuf[u]);
						if (chbeg <= pixels && pixels < str_px)
						{
							pos = static_cast<unsigned>(u);
							if ((pxbuf[u] <= 1) || (pixels <= chbeg + static_cast<int>(pxbuf[u] >> 1)))
								++pos;
							break;
						}
						str_px -= pxbuf[u];
					}
				}
				else
				{
					//LTR
					for (std::size_t u = 0; u < len; ++u)
					{
						if (pixels < static_cast<int>(pxbuf[u]))
						{
							pos = static_cast<unsigned>(u);
							if ((pxbuf[u] > 1) && (pixels > static_cast<int>(pxbuf[u] >> 1)))
								++pos;

							break;
						}
						pixels -= static_cast<int>(pxbuf[u]);
					}
				}
			}
			return pos;
		}

		unsigned text_editor::_m_pixels_by_char(const nana::string& lnstr, std::size_t pos) const
		{
			unicode_bidi bidi;
			std::vector<unicode_bidi::entity> reordered;
			bidi.linestr(lnstr.c_str(), lnstr.size(), reordered);

			const nana::char_t * ch = (pos <= lnstr.size() ? lnstr.c_str() + pos : 0);

			std::size_t pxbuf_size = 0;
			std::unique_ptr<unsigned[]> pxbuf;

			unsigned text_w = 0;
			for (auto & ent : reordered)
			{
				std::size_t len = ent.end - ent.begin;
				if (ent.begin <= ch && ch <= ent.end)
				{
					if (_m_is_right_text(ent))
					{
						//RTL
						if (len > pxbuf_size)
						{
							pxbuf.reset(new unsigned[len]);
							pxbuf_size = len;
						}

						graph_.glyph_pixels(ent.begin, len, pxbuf.get());
						unsigned * end = pxbuf.get() + len;
						for (unsigned * u = pxbuf.get() + (ch - ent.begin); u != end; ++u)
							text_w += *u;
					}
					else
					{
						//LTR
						text_w += _m_text_extent_size(ent.begin, ch - ent.begin).width;
					}
					break;
				}
				else
					text_w += _m_text_extent_size(ent.begin, len).width;
			}
			return text_w;
		}

		bool text_editor::_m_is_right_text(const unicode_bidi::entity& e)
		{
			if(e.bidi_char_type == unicode_bidi::bidi_char::L)
				return false;
			return (e.level & 1);
		}

		//struct attributes
			text_editor::attributes::attributes()
				:	line_wrapped(false),
					multi_lines(true),
					editable(true),
					enable_background(true),
					enable_counterpart(false)
			{}
		//end struct attributes

		//struct coordinate
			text_editor::coordinate::coordinate():xpos(0){}
		//end struct coordinate
		//end class text_editor
	}//end namespace skeletons
}//end namespace widgets
}//end namespace nana

