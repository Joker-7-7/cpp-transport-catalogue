#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    catalogue::Transport catalogue;
    reader::Input reader;
    reader.ReadFrom(cin, catalogue);
    reader::ReadRequests(cin, catalogue, cout);
}
