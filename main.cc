
#include "system.hh"
#include <memory>

int main() {

auto system = std::make_unique<System>();

system->setup(65536);

for (int i = 0; i < 100; i++) {
	system->advance(1.0f);
	//system->write_points(i);
}

}