#include <iostream>
#include <fstream>
#include <optional>
#include <cstring>

// todo replace strtod to from_chars
#include <cmath>
#include <cstdlib>

struct program_args {
	int argc;
	const char** argv;

	program_args(int c, const char** v)
		: argc{c}
		, argv{v}
	{
	}
};

void print_usage(program_args args)
{
	const auto prg_name = [&] {
		if(args.argc == 1) return args.argv[0];
		return "./bri";
	}();

	std::cout << "Usage:\n";
	std::cout << prg_name << " float-value\n\n";
	std::cout << "float-value must be between 0 and 100.\n";
}

std::optional<double> chartodouble(const char* begin, const char* end)
{
	char* vend;
	auto value = std::strtod(begin, &vend);

	if(vend == begin || value == HUGE_VAL) {
		return std::nullopt;
	}

	return value;
}

void set_brightness(double value, const char* device);

int main(const int argc, const char** argv)
{
	if(argc != 2) {
		std::cout << "Missing argument(s).\n";
		print_usage({argc, argv});
		return 1;
	}

	auto value_opt = chartodouble(argv[1], argv[1] + std::strlen(argv[1]));

	if(!value_opt.has_value()) {
		std::cout << "Bad float value.\n";
		print_usage({argc, argv});
		return 2;
	}

	auto value = value_opt.value();

	try {
		set_brightness(value, "intel_backlight");
	} catch(const std::exception& ex) {
		std::cerr << "exception: " << ex.what() << '\n';
	}
}

int get_max_brightness(const std::string& base_path)
{
	const auto path = base_path + "/max_brightness";
	int value = 0;

	std::ifstream in{path};
	if(!in.is_open()) {
		throw std::runtime_error{"could not open max_brightness (" + path + ")."};
	}

	in >> value;

	if(in.bad()) {
		throw std::runtime_error{"could not read max_brightness (" + path + ")."};
	}

	return value;
}

void set_brightness(const double value, const char* const device)
{
	const auto base_path = "/sys/class/backlight/" + std::string{device};
	if(value < 0.0 || value > 100.0) {
		throw std::out_of_range{"brightness value is out of range."};
	}

	auto max = get_max_brightness(base_path);

	const auto path = base_path + "/brightness";

	std::ofstream out{path};

	if(!out.is_open()) {
		throw std::runtime_error{"could not open brightness (" + path + ")."};
	}

	const auto real_value = static_cast<int>(value * max / 100.0);
	std::cout << real_value << '\n';

	out << real_value;

	if(out.bad()) {
		throw std::runtime_error{"could not write brightness (" + path + ")."};
	}
}
