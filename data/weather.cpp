/**
 *  @example puddle/example/weather.cpp
 *  This example shows the usage of the Puddle compile-time layer.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <puddle/puddle.hpp>

#include <type_traits>
#include <iostream>

enum class month_name {
	january, february, march, april, may, june,
	july, august, september, october, november, december
};

namespace weather {

enum class type
{
	sunny, clear, mostly_sunny, partly_sunny, partly_cloudy, mostly_cloudy,
	cloudy, overcast, storm, thunderstorm, snowstorm, mist, fog,
	light_rain, rain, showers, snow_showers, snow, hail
};

enum class temperature_unit
{
	degrees_celsius, degrees_fahrenheit
};

enum class speed_unit
{
	meters_per_second, kilometers_per_second, miles_per_hour,
	knots
};

enum class cardinal_direction
{
	N, S, E, W, NE, NW, SE, SW, NNE, ENE, NNW, WNW, SSE, ESE, SSW, WSW
};

struct temperature
{
	double value;
	temperature_unit units;
};

struct wind_info
{
	double avg_speed;
	double max_speed;
	speed_unit units;
	cardinal_direction direction;
	bool high_wind_warning;
};

struct status
{
	type conditions;
	temperature min_temperature, max_temperature;
	double humidity;
	wind_info wind;
};

struct forecast
{
	int year;
	month_name month;
	int day;
	std::string place;
	status morning, afternoon, evening, night;
};

forecast today = {
	2011, month_name::april, 10,
	"Tatry",
	{
		type::mostly_cloudy,
		{3.0, temperature_unit::degrees_celsius},
		{7.0, temperature_unit::degrees_celsius},
		41.0,
		{25.0, 40.0, speed_unit::meters_per_second, cardinal_direction::N, true}
	},
	{
		type::overcast,
		{4.0, temperature_unit::degrees_celsius},
		{9.0, temperature_unit::degrees_celsius},
		44.0,
		{20.0, 35.0, speed_unit::meters_per_second, cardinal_direction::N, true}
	},
	{
		type::rain,
		{7.0, temperature_unit::degrees_celsius},
		{13.0, temperature_unit::degrees_celsius},
		47.0,
		{15.0, 30.0, speed_unit::meters_per_second, cardinal_direction::NNW, true}
	},
	{
		type::snow,
		{0.0, temperature_unit::degrees_celsius},
		{4.0, temperature_unit::degrees_celsius},
		52.0,
		{5.0, 10.0, speed_unit::meters_per_second, cardinal_direction::NW, false}
	}
};

forecast tomorrow = {
	2011, month_name::april, 11,
	"Tatry",
	{
		type::snow_showers,
		{-1.0, temperature_unit::degrees_celsius},
		{2.0, temperature_unit::degrees_celsius},
		50.0,
		{5.0, 10.0, speed_unit::meters_per_second, cardinal_direction::NW, false}
	},
	{
		type::showers,
		{3.0, temperature_unit::degrees_celsius},
		{7.0, temperature_unit::degrees_celsius},
		50.0,
		{2.0, 5.0, speed_unit::meters_per_second, cardinal_direction::WNW, false}
	},
	{
		type::mostly_cloudy,
		{7.0, temperature_unit::degrees_celsius},
		{12.0, temperature_unit::degrees_celsius},
		47.0,
		{3.0, 7.0, speed_unit::meters_per_second, cardinal_direction::W, false}
	},
	{
		type::partly_cloudy,
		{2.0, temperature_unit::degrees_celsius},
		{5.0, temperature_unit::degrees_celsius},
		40.0,
		{5.0, 10.0, speed_unit::meters_per_second, cardinal_direction::W, false}
	}
};


} // namespace weather

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_ENUM(month_name,
	(january)(february)(march)(april)(may)(june)
	(july)(august)(september)(october)(november)(december)
)

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(weather)

MIRROR_QREG_ENUM(weather, type,
	(sunny)(clear)(mostly_sunny)(partly_sunny)(partly_cloudy)(mostly_cloudy)
	(cloudy)(overcast)(storm)(thunderstorm)(snowstorm)(mist)(fog)
	(light_rain)(rain)(showers)(snow_showers)(snow)(hail)
)

MIRROR_QREG_ENUM(weather, temperature_unit,
	(degrees_celsius)(degrees_fahrenheit)
)

MIRROR_QREG_ENUM(weather, speed_unit,
	(meters_per_second)(kilometers_per_second)(miles_per_hour)
	(knots)
)

MIRROR_QREG_ENUM(weather, cardinal_direction,
	(N)(S)(E)(W)(NE)(NW)(SE)(SW)(NNE)(ENE)(NNW)(WNW)(SSE)(ESE)(SSW)(WSW)
)

MIRROR_QREG_POD_CLASS(
	struct, weather, temperature,
	(value)(units)
)

MIRROR_QREG_POD_CLASS(
	struct, weather, wind_info,
	(avg_speed)
	(max_speed)
	(units)
	(direction)
	(high_wind_warning)
)

MIRROR_QREG_POD_CLASS(
	struct, weather, status,
	(conditions)
	(min_temperature)
	(max_temperature)
	(humidity)
	(wind)
)

MIRROR_QREG_POD_CLASS(
	struct, weather, forecast,
	(year)(month)(day)
	(place)
	(morning)(afternoon)(evening)(night)
)

MIRROR_QREG_FREE_VARIABLES(weather, (today)(tomorrow))

MIRROR_REG_END

class json_printer
{
private:
	int indent_level;

	std::ostream& indented(std::ostream& out) const
	{
		for(int i=0;i!=indent_level;++i) out << "  ";
		return out;
	}


	template <typename T>
	static std::true_type is_writable(
		T*,
		decltype(std::cout << (*((T*)nullptr)) == std::cout) = false
	);
	static std::false_type is_writable(...);

	// checks if T is directly writable to std::cout
	template <typename T>
	static auto writable(const T&) -> decltype(is_writable((T*)nullptr));

	// prints types directly writable to ostream
	template <typename MetaObject>
	void do_print(
		MetaObject mo,
		mirror::meta_type_tag,
		std::true_type
	) const
	{
		std::cout << "'" << mo.get() << "'";
	}

	// prints enumerated type values
	template <typename MetaObject>
	void do_print(
		MetaObject mo,
		mirror::meta_enum_tag,
		std::false_type
	) const
	{
		std::cout << "'" << mo.value_name() << "'";
	}

	// prints elaborated type values which are not directly writable to cout
	template <typename MetaObject>
	void do_print(
		MetaObject mo,
		mirror::meta_class_tag,
		std::false_type
	) const
	{
		if(mo.attributes().empty()) std::cout << "N/A";
		else
		{
			std::cout << "{" << std::endl;
			mo.attributes().for_each(json_printer(indent_level+1));
			indented(std::cout) << "}";
		}
	}

	// prints a variable
	template <typename MetaVariable>
	void print(MetaVariable mv, mirror::meta_variable_tag) const
	{
		indented(std::cout) << "'" << mv.base_name() << "': ";
		std::false_type wr;
		auto cat = mv.type().category();
		do_print(puddle::instance_of(mv), cat, wr);
	}

	// prints an instance
	template <typename MetaInstance>
	void print(MetaInstance mi, puddle::meta_instance_tag) const
	{
		indented(std::cout)<< "'" << mi.variable().base_name() << "': ";
		decltype(writable(mi.get())) wr;
		auto cat = mi.variable().type().category();
		do_print(mi, cat, wr);
	}
public:
	json_printer(int indent = 0)
	 : indent_level(indent)
	{ }

	template <typename MetaObject>
	void operator()(MetaObject mo, bool first, bool last) const
	{
		print(mo, mo.category());
		if(!last) std::cout << ",";
		std::cout << std::endl;
	}
};


int main(void)
{
	auto meta_weather = puddle::adapt<MIRRORED_NAMESPACE(weather)>();
	meta_weather.free_variables().for_each(json_printer());
	std::cout << std::endl;
	return 0;
}

/* Example of output:
 | 'today': {
 |   'year': '2011',
 |   'month': 'april',
 |   'day': '10',
 |   'place': 'Tatry',
 |   'morning': {
 |     'conditions': 'mostly_cloudy',
 |     'min_temperature': {
 |       'value': '3',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '7',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '41',
 |     'wind': {
 |       'avg_speed': '25',
 |       'max_speed': '40',
 |       'units': 'meters_per_second',
 |       'direction': 'N',
 |       'high_wind_warning': '1'
 |     }
 |   },
 |   'afternoon': {
 |     'conditions': 'overcast',
 |     'min_temperature': {
 |       'value': '4',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '9',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '44',
 |     'wind': {
 |       'avg_speed': '20',
 |       'max_speed': '35',
 |       'units': 'meters_per_second',
 |       'direction': 'N',
 |       'high_wind_warning': '1'
 |     }
 |   },
 |   'evening': {
 |     'conditions': 'rain',
 |     'min_temperature': {
 |       'value': '7',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '13',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '47',
 |     'wind': {
 |       'avg_speed': '15',
 |       'max_speed': '30',
 |       'units': 'meters_per_second',
 |       'direction': 'NNW',
 |       'high_wind_warning': '1'
 |     }
 |   },
 |   'night': {
 |     'conditions': 'snow',
 |     'min_temperature': {
 |       'value': '0',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '4',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '52',
 |     'wind': {
 |       'avg_speed': '5',
 |       'max_speed': '10',
 |       'units': 'meters_per_second',
 |       'direction': 'NW',
 |       'high_wind_warning': '0'
 |     }
 |   }
 | },
 | 'tomorrow': {
 |   'year': '2011',
 |   'month': 'april',
 |   'day': '11',
 |   'place': 'Tatry',
 |   'morning': {
 |     'conditions': 'snow_showers',
 |     'min_temperature': {
 |       'value': '-1',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '2',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '50',
 |     'wind': {
 |       'avg_speed': '5',
 |       'max_speed': '10',
 |       'units': 'meters_per_second',
 |       'direction': 'NW',
 |       'high_wind_warning': '0'
 |     }
 |   },
 |   'afternoon': {
 |     'conditions': 'showers',
 |     'min_temperature': {
 |       'value': '3',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '7',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '50',
 |     'wind': {
 |       'avg_speed': '2',
 |       'max_speed': '5',
 |       'units': 'meters_per_second',
 |       'direction': 'WNW',
 |       'high_wind_warning': '0'
 |     }
 |   },
 |   'evening': {
 |     'conditions': 'mostly_cloudy',
 |     'min_temperature': {
 |       'value': '7',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '12',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '47',
 |     'wind': {
 |       'avg_speed': '3',
 |       'max_speed': '7',
 |       'units': 'meters_per_second',
 |       'direction': 'W',
 |       'high_wind_warning': '0'
 |     }
 |   },
 |   'night': {
 |     'conditions': 'partly_cloudy',
 |     'min_temperature': {
 |       'value': '2',
 |       'units': 'degrees_celsius'
 |     },
 |     'max_temperature': {
 |       'value': '5',
 |       'units': 'degrees_celsius'
 |     },
 |     'humidity': '40',
 |     'wind': {
 |       'avg_speed': '5',
 |       'max_speed': '10',
 |       'units': 'meters_per_second',
 |       'direction': 'W',
 |       'high_wind_warning': '0'
 |     }
 |   }
 | }
 */
