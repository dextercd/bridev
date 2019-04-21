#include <iostream>
#include <fstream>
#include <optional>
#include <cstring>

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

void print_usage(const program_args args)
{
	const auto prg_name = [&] {
		if(args.argc >= 1) return args.argv[0];
		return "./bri";
	}();

	std::cout << "Usage:\n";
	std::cout << prg_name << " float-value\n\n";
	std::cout << "float-value must be between 0 and 100.\n";
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

void set_brightness(double brightness, const char* device);

int main(const int argc, const char** const argv)
{
	if(argc != 2) {
		std::cout << "Missing argument(s).\n";
		print_usage({argc, argv});
		return 1;
	}

	auto brightness_opt = chartodouble(argv[1], argv[1] + std::strlen(argv[1]));

	if(!brightness_opt.has_value()) {
		std::cout << "Bad float brightness.\n";
		print_usage({argc, argv});
		return 2;
	}

	auto brightness = brightness_opt.value();

	try {
		set_brightness(brightness, "intel_backlight");
	} catch(const std::exception& ex) {
		std::cerr << "exception: " << ex.what() << '\n';
		return 3;
	} catch(...) {
		std::cerr << "unknown exception\n";
		return 4;
	}
}

int get_max_brightness(const std::string& base_path)
{
	const auto path = base_path + "/max_brightness";

	std::ifstream in{path};
	if(!in.is_open()) {
		throw std::runtime_error{"could not open max_brightness (" + path + ")."};
	}

	int value = 0;
	in >> value;

	if(in.fail() || in.eof()) {
		throw std::runtime_error{"could not read max_brightness (" + path + ")."};
	}

	return value;
}

void set_brightness(const double brightness, const char* const device)
{
	const auto base_path = "/sys/class/backlight/" + std::string{device};

	if(std::isnan(brightness))
		throw std::out_of_range{"brightness brightness is NaN."};

	if(std::isinf(brightness))
		throw std::out_of_range{"brightness value is INF."};

	if(brightness < 0.0 || brightness > 100.0)
		throw std::out_of_range{"brightness value is out of range."};

	const auto max = get_max_brightness(base_path);

	const auto path = base_path + "/brightness";

	std::ofstream out{path};

	if(!out.is_open()) {
		throw std::runtime_error{"could not open brightness (" + path + ")."};
	}

	const auto real_brightness = static_cast<int>(brightness * max / 100.0);
	std::cout << real_brightness << '\n';

	out << real_brightness;

	if(out.fail() || out.eof()) {
		throw std::runtime_error{"could not write brightness (" + path + ")."};
	}
}
