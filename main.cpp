#include <iostream>
#include <fstream>
#include <optional>
#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;

// todo replace strtod to from_chars when available in gcc
#include <cmath>
#include <cstdlib>

struct program_args {
	int argc;
	const char** argv;

	program_args(const int c, const char** const v)
		: argc{c}
		, argv{v}
	{
	}
};

enum main_return_code {
	success,
	missing_arguments,
	could_not_parse,
	exception,
	unknown_exception,
	bad_brightness,
};

void print_usage(const program_args args)
{
	const auto prg_name = [&] {
		if(args.argc >= 1) return args.argv[0];
		return "./bri";
	}();

	std::cout << "Usage:\n";
	std::cout << prg_name << " device-name float-value\n\n";
	std::cout << "device-name: a device in /sys/class/backlight.\n";
	std::cout << "float-value: brightness value between 0 and 100.\n";
}

std::optional<double> chartodouble(const char* const begin, const char* const end)
{
	char* vend;
	const auto value = std::strtod(begin, &vend);

	if(vend == begin || value == HUGE_VAL) {
		return std::nullopt;
	}

	return value;
}

void set_brightness(double brightness, const fs::path& base_path);

int main(const int argc, const char** const argv)
{
	if(argc != 3) {
		std::cout << "Missing argument(s).\n";
		print_usage({argc, argv});
		return main_return_code::missing_arguments;
	}

	auto device_name = argv[1];
	auto brightness_opt = chartodouble(argv[2], argv[2] + std::strlen(argv[1]));

	if(!brightness_opt.has_value()) {
		std::cout << "Couldn't parse brightness value.\n";
		print_usage({argc, argv});
		return main_return_code::could_not_parse;
	}

	auto brightness = brightness_opt.value();

	if(std::isnan(brightness) || std::isinf(brightness) || brightness < 0.0 || brightness > 100.0) {
		std::cout << "Bad brightness value.\n";
		print_usage({argc, argv});
		return main_return_code::bad_brightness;
	}


	try {
		const auto class_path = fs::path{"/sys/class/backlight"};
		const auto base_path = class_path / device_name;

		if(base_path.parent_path() != class_path) {
			throw std::runtime_error{"non-relative device name."};
		}

		if(!fs::exists(base_path)) {
			throw std::runtime_error{base_path.string() + " does not exist."};
		}

		// all checks succeeded, set the brightness
		set_brightness(brightness, base_path);
	} catch(const std::exception& ex) {
		std::cerr << "exception: " << ex.what() << '\n';
		return main_return_code::exception;
	} catch(...) {
		std::cerr << "unknown exception\n";
		return main_return_code::unknown_exception;
	}
}

int get_max_brightness(const fs::path& base_path)
{
	const auto path = base_path / "max_brightness";

	std::ifstream in{path};
	if(!in.is_open()) {
		throw std::runtime_error{"could not open max_brightness (" + path.string() + ")."};
	}

	int value = 0;
	in >> value;

	if(in.fail() || in.eof()) {
		throw std::runtime_error{"could not read max_brightness (" + path.string() + ")."};
	}

	return value;
}

void set_brightness(const double brightness, const fs::path& base_path)
{
	const auto max = get_max_brightness(base_path);

	const auto path = base_path / "brightness";

	std::ofstream out{path};

	if(!out.is_open()) {
		throw std::runtime_error{"could not open brightness (" + path.string() + ")."};
	}

	const auto real_brightness = static_cast<int>(brightness * max / 100.0);
	std::cout << real_brightness << '\n';

	out << real_brightness;

	if(out.fail() || out.eof()) {
		throw std::runtime_error{"could not write brightness (" + path.string() + ")."};
	}
}
