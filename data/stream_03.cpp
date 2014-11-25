/**
 *  @example mirror/example/stream_03.cpp
 *  This example shows the registering macros, container reflection
 *  and wrapper classes which allow to write instances of arbitrary
 *  reflectible class to standard output streams in the JSON format
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/type/native.hpp>
#include <mirror/pre_registered/type/std/string.hpp>
#include <mirror/pre_registered/class/std/list.hpp>
#include <mirror/pre_registered/class/boost/optional.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <mirror/stream/json.hpp>
#include <iostream>

namespace music {

enum class genre
{
	rock, alt_rock, hard_rock, rap, hip_hop, pop, jazz
};

struct album
{
	std::string title;
	short year;

	std::list<std::string> tracks;
};

struct band_membership
{
	short year_from;
	boost::optional<short> year_to;

	std::list<std::string> roles;
};

struct musican
{
	boost::optional<std::string> nickname;

	std::string first_name;
	std::string family_name;
	std::list<band_membership> membership;
};

struct band
{
	std::string name;

	std::list<genre> genres;
	std::list<musican> members;
	std::list<album> albums;
};

} // namespace music

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(music)

MIRROR_QREG_ENUM(
	music, genre,
	(rock)(alt_rock)(hard_rock)(rap)(hip_hop)(pop)(jazz)
)

MIRROR_QREG_POD_CLASS(
	struct, music, album,
	(title)(year)(tracks)
)

MIRROR_QREG_POD_CLASS(
	struct, music, band_membership,
	(year_from)(year_to)(roles)
)

MIRROR_QREG_POD_CLASS(
	struct, music, musican,
	(nickname)(first_name)(family_name)(membership)
)

MIRROR_QREG_POD_CLASS(
	struct, music, band,
	(name)(genres)(members)(albums)
)

MIRROR_REG_END

int main(void)
{
	boost::none_t none = boost::none_t();
	using namespace mirror;
	//
	std::list<music::band> bands = {
	{
		"Radiohead",
		{music::genre::alt_rock},
		{
			{none, "Thom", "Yorke", {{1985, none, {"vocals", "guitar", "piano"}}}},
			{none, "Johnny", "Greenwood", {{1985, none, {"guitar", "keyboards", "other instruments"}}}},
			{none, "Ed", "O'Brien", {{1985, none, {"guitar", "backing vocals"}}}},
			{none, "Colin", "Greenwood", {{1985, none, {"bass", "synthesizers"}}}},
			{none, "Phil", "Selway", {{1985, none, {"drums", "percussion"}}}}
		},
		{
			{"Pablo Honey", 1993, {"You", "Creep", "..."}},
			{"The Bends", 1995, {"Planet Telex", "The Bends", "..."}},
			{"Ok Computer", 1997, {"Airbag", "Paranoid Android", "..."}},
			{"Kid A", 2000, {"Everything in Its Right Place", "..."}},
			{"Amnesiac", 2001, {"Packt Like Sardines in a Crushd Tin Box", "..."}},
			{"Hail to the Thief", 2003, {"2 + 2 = 5", "..."}},
			{"In Rainbows", 2007, {"15 Step", "Bodysnatchers", "..."}},
			{"The King of Limbs", 2011, {"Bloom", "Morning Mr. Magpie", "..."}}
		}
	},
	{
		"U2",
		{music::genre::rock, music::genre::pop},
		{
			{std::string("Bono Vox"), "Paul", "Hewson", {{1971, none, {"vocals", "guitar"}}}},
			{std::string("The Edge"), "David", "Evans", {{1971, none, {"guitar", "keyboards", "vocals"}}}},
			{none, "Adam", "Clayton", {{1971, none, {"bass guitar"}}}},
			{none, "Larry Jr.", "Mullen", {{1971, none, {"drums", "percussion"}}}}
		},
		{
			{"Boy", 1980, {"I will follow", "Twilight", "..."}},
			{"October", 1981, {"Gloria", "I fall down", "..."}},
			{"War", 1983, {"Sunday Bloody Sunday", "Seconds", "New Year's Day", "..."}},
			{"The Unforgettable Fire", 1984, {"A Sort of homecomming", "..."}},
			{"The Joshua Tree", 1987, {"Where the Streets Have No Name", "..."}},
			{"Rattle and Hum", 1988, {"Helter Skelter", "Van Diemen's Land", "..."}},
			{"Achtung Baby", 1991, {"Zoo Station", "Even Better Than the Real Thing", "..."}},
			{"Zooropa", 1993, {"Zooropa", "Babyface", "..."}},
			{"Pop", 1997, {u8"Discothèque", "..."}},
			{"All That You Can't Leave Behind", 2000, {"Beautiful Day", "..."}},
			{"How to Dismantle an Atomic Bomb", 2004, {"Vertigo", "Miracle Drug", "..."}},
			{"No Line on the Horizon", 2009, {"No Line on the Horizon", "..."}}
		}
	},
	{
		"Rage Against the Machine",
		{music::genre::rock, music::genre::rap},
		{
			{none, "Zack", "de la Rocha", {{1991, 2000, {"vocals"}},{2007, none, {"vocals"}}}},
			{none, "Tom", "Morello", {{1991, 2000, {"guitar"}},{2007, none, {"guitar"}}}},
			{none, "Tim", "Commerford", {{1992, 2000, {"bass", "backing vocals"}},{2007, none, {"bass"}}}},
			{none, "Brad", "Wilk", {{1991, 2000, {"drums"}},{2007, none, {"drums"}}}}
		},
		{
			{"Rage Against the Machine", 1992, {"Bombtrack", "Killing in the Name", "..."}},
			{"Evil Empire", 1996, {"People of the Sun", "Bulls on Parade", "..."}},
			{"The Battle of Los Angeles", 1999, {"Testify", "Guerrilla Radio", "..."}},
			{"Renegades", 2000, {"Microphone Fiend", "..."}}
		}
	},
	{
		"Audioslave",
		{music::genre::rock},
		{
			{none, "Chris", "Cornell", {{2000, 2007, {"vocals", "rythm guitar"}}}},
			{none, "Tom", "Morello", {{2000, 2007, {"guitar"}}}},
			{none, "Tim", "Commerford", {{2000, 2007, {"bass guitar", "backing vocals"}}}},
			{none, "Brad", "Wilk", {{2000, 2007, {"drums"}}}}
		},
		{
			{"Audioslave", 2002, {"Cochise", "Show Me How to Live", "Gasoline", "What You Are", "Like a Stone", "..."}},
			{"Out of Exile", 2005, {"Your Time Has Come", "Out of Exile", "..."}},
			{"Revelations", 2006, {"Revelations", "One and the Same", "..."}}
		}
	}
	};
	auto name_maker = [](std::ostream& out){out << "bands";};
	// write in JSON format
	std::cout << stream::to_json::from(bands, name_maker) << std::endl;
	//
	return 0;
}

/* Example of output:
 |      "bands": [
 |          {
 |              "name": "Radiohead",
 |              "genres": [
 |                  "alt_rock"
 |              ],
 |              "members": [
 |                  {
 |                      "nickname": nil,
 |                      "first_name": "Thom",
 |                      "family_name": "Yorke",
 |                      "membership": [
 |                          {
 |                              "year_from": 1985,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "vocals", "guitar", "piano"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Johnny",
 |                      "family_name": "Greenwood",
 |                      "membership": [
 |                          {
 |                              "year_from": 1985,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "guitar", "keyboards", "other instruments"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Ed",
 |                      "family_name": "O\'Brien",
 |                      "membership": [
 |                          {
 |                              "year_from": 1985,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "guitar", "backing vocals"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Colin",
 |                      "family_name": "Greenwood",
 |                      "membership": [
 |                          {
 |                              "year_from": 1985,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "bass", "synthesizers"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Phil",
 |                      "family_name": "Selway",
 |                      "membership": [
 |                          {
 |                              "year_from": 1985,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "drums", "percussion"
 |                              ]
 |                          }
 |                      ]
 |                  }
 |              ],
 |              "albums": [
 |                  {
 |                      "title": "Pablo Honey",
 |                      "year": 1993,
 |                      "tracks": [
 |                          "You", "Creep", "..."
 |                      ]
 |                  }, {
 |                      "title": "The Bends",
 |                      "year": 1995,
 |                      "tracks": [
 |                          "Planet Telex", "The Bends", "..."
 |                      ]
 |                  }, {
 |                      "title": "Ok Computer",
 |                      "year": 1997,
 |                      "tracks": [
 |                          "Airbag", "Paranoid Android", "..."
 |                      ]
 |                  }, {
 |                      "title": "Kid A",
 |                      "year": 2000,
 |                      "tracks": [
 |                          "Everything in Its Right Place", "..."
 |                      ]
 |                  }, {
 |                      "title": "Amnesiac",
 |                      "year": 2001,
 |                      "tracks": [
 |                          "Packt Like Sardines in a Crushd Tin Box", "..."
 |                      ]
 |                  }, {
 |                      "title": "Hail to the Thief",
 |                      "year": 2003,
 |                      "tracks": [
 |                          "2 + 2 = 5", "..."
 |                      ]
 |                  }, {
 |                      "title": "In Rainbows",
 |                      "year": 2007,
 |                      "tracks": [
 |                          "15 Step", "Bodysnatchers", "..."
 |                      ]
 |                  }, {
 |                      "title": "The King of Limbs",
 |                      "year": 2011,
 |                      "tracks": [
 |                          "Bloom", "Morning Mr. Magpie", "..."
 |                      ]
 |                  }
 |              ]
 |          }, {
 |              "name": "U2",
 |              "genres": [
 |                  "rock", "pop"
 |              ],
 |              "members": [
 |                  {
 |                      "nickname": "Bono Vox",
 |                      "first_name": "Paul",
 |                      "family_name": "Hewson",
 |                      "membership": [
 |                          {
 |                              "year_from": 1971,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "vocals", "guitar"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": "The Edge",
 |                      "first_name": "David",
 |                      "family_name": "Evans",
 |                      "membership": [
 |                          {
 |                              "year_from": 1971,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "guitar", "keyboards", "vocals"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Adam",
 |                      "family_name": "Clayton",
 |                      "membership": [
 |                          {
 |                              "year_from": 1971,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "bass guitar"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Larry Jr.",
 |                      "family_name": "Mullen",
 |                      "membership": [
 |                          {
 |                              "year_from": 1971,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "drums", "percussion"
 |                              ]
 |                          }
 |                      ]
 |                  }
 |              ],
 |              "albums": [
 |                  {
 |                      "title": "Boy",
 |                      "year": 1980,
 |                      "tracks": [
 |                          "I will follow", "Twilight", "..."
 |                      ]
 |                  }, {
 |                      "title": "October",
 |                      "year": 1981,
 |                      "tracks": [
 |                          "Gloria", "I fall down", "..."
 |                      ]
 |                  }, {
 |                      "title": "War",
 |                      "year": 1983,
 |                      "tracks": [
 |                          "Sunday Bloody Sunday", "Seconds", "New Year\'s Day", "..."
 |                      ]
 |                  }, {
 |                      "title": "The Unforgettable Fire",
 |                      "year": 1984,
 |                      "tracks": [
 |                          "A Sort of homecomming", "..."
 |                      ]
 |                  }, {
 |                      "title": "The Joshua Tree",
 |                      "year": 1987,
 |                      "tracks": [
 |                          "Where the Streets Have No Name", "..."
 |                      ]
 |                  }, {
 |                      "title": "Rattle and Hum",
 |                      "year": 1988,
 |                      "tracks": [
 |                          "Helter Skelter", "Van Diemen\'s Land", "..."
 |                      ]
 |                  }, {
 |                      "title": "Achtung Baby",
 |                      "year": 1991,
 |                      "tracks": [
 |                          "Zoo Station", "Even Better Than the Real Thing", "..."
 |                      ]
 |                  }, {
 |                      "title": "Zooropa",
 |                      "year": 1993,
 |                      "tracks": [
 |                          "Zooropa", "Babyface", "..."
 |                      ]
 |                  }, {
 |                      "title": "Pop",
 |                      "year": 1997,
 |                      "tracks": [
 |                          "Discothèque", "..."
 |                      ]
 |                  }, {
 |                      "title": "All That You Can\'t Leave Behind",
 |                      "year": 2000,
 |                      "tracks": [
 |                          "Beautiful Day", "..."
 |                      ]
 |                  }, {
 |                      "title": "How to Dismantle an Atomic Bomb",
 |                      "year": 2004,
 |                      "tracks": [
 |                          "Vertigo", "Miracle Drug", "..."
 |                      ]
 |                  }, {
 |                      "title": "No Line on the Horizon",
 |                      "year": 2009,
 |                      "tracks": [
 |                          "No Line on the Horizon", "..."
 |                      ]
 |                  }
 |              ]
 |          }, {
 |              "name": "Rage Against the Machine",
 |              "genres": [
 |                  "rock", "rap"
 |              ],
 |              "members": [
 |                  {
 |                      "nickname": nil,
 |                      "first_name": "Zack",
 |                      "family_name": "de la Rocha",
 |                      "membership": [
 |                          {
 |                              "year_from": 1991,
 |                              "year_to": 2000,
 |                              "roles": [
 |                                  "vocals"
 |                              ]
 |                          }, {
 |                              "year_from": 2007,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "vocals"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Tom",
 |                      "family_name": "Morello",
 |                      "membership": [
 |                          {
 |                              "year_from": 1991,
 |                              "year_to": 2000,
 |                              "roles": [
 |                                  "guitar"
 |                              ]
 |                          }, {
 |                              "year_from": 2007,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "guitar"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Tim",
 |                      "family_name": "Commerford",
 |                      "membership": [
 |                          {
 |                              "year_from": 1992,
 |                              "year_to": 2000,
 |                              "roles": [
 |                                  "bass", "backing vocals"
 |                              ]
 |                          }, {
 |                              "year_from": 2007,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "bass"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Brad",
 |                      "family_name": "Wilk",
 |                      "membership": [
 |                          {
 |                              "year_from": 1991,
 |                              "year_to": 2000,
 |                              "roles": [
 |                                  "drums"
 |                              ]
 |                          }, {
 |                              "year_from": 2007,
 |                              "year_to": nil,
 |                              "roles": [
 |                                  "drums"
 |                              ]
 |                          }
 |                      ]
 |                  }
 |              ],
 |              "albums": [
 |                  {
 |                      "title": "Rage Against the Machine",
 |                      "year": 1992,
 |                      "tracks": [
 |                          "Bombtrack", "Killing in the Name", "..."
 |                      ]
 |                  }, {
 |                      "title": "Evil Empire",
 |                      "year": 1996,
 |                      "tracks": [
 |                          "People of the Sun", "Bulls on Parade", "..."
 |                      ]
 |                  }, {
 |                      "title": "The Battle of Los Angeles",
 |                      "year": 1999,
 |                      "tracks": [
 |                          "Testify", "Guerrilla Radio", "..."
 |                      ]
 |                  }, {
 |                      "title": "Renegades",
 |                      "year": 2000,
 |                      "tracks": [
 |                          "Microphone Fiend", "..."
 |                      ]
 |                  }
 |              ]
 |          }, {
 |              "name": "Audioslave",
 |              "genres": [
 |                  "rock"
 |              ],
 |              "members": [
 |                  {
 |                      "nickname": nil,
 |                      "first_name": "Chris",
 |                      "family_name": "Cornell",
 |                      "membership": [
 |                          {
 |                              "year_from": 2000,
 |                              "year_to": 2007,
 |                              "roles": [
 |                                  "vocals", "rythm guitar"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Tom",
 |                      "family_name": "Morello",
 |                      "membership": [
 |                          {
 |                              "year_from": 2000,
 |                              "year_to": 2007,
 |                              "roles": [
 |                                  "guitar"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Tim",
 |                      "family_name": "Commerford",
 |                      "membership": [
 |                          {
 |                              "year_from": 2000,
 |                              "year_to": 2007,
 |                              "roles": [
 |                                  "bass guitar", "backing vocals"
 |                              ]
 |                          }
 |                      ]
 |                  }, {
 |                      "nickname": nil,
 |                      "first_name": "Brad",
 |                      "family_name": "Wilk",
 |                      "membership": [
 |                          {
 |                              "year_from": 2000,
 |                              "year_to": 2007,
 |                              "roles": [
 |                                  "drums"
 |                              ]
 |                          }
 |                      ]
 |                  }
 |              ],
 |              "albums": [
 |                  {
 |                      "title": "Audioslave",
 |                      "year": 2002,
 |                      "tracks": [
 |                          "Cochise", "Show Me How to Live", "Gasoline", "What You Are", "Like a Stone", "..."
 |                      ]
 |                  }, {
 |                      "title": "Out of Exile",
 |                      "year": 2005,
 |                      "tracks": [
 |                          "Your Time Has Come", "Out of Exile", "..."
 |                      ]
 |                  }, {
 |                      "title": "Revelations",
 |                      "year": 2006,
 |                      "tracks": [
 |                          "Revelations", "One and the Same", "..."
 |                      ]
 |                  }
 |              ]
 |          }
 |      ]
 */
