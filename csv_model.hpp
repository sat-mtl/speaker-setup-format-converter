#pragma once
#include <string>
#include <variant>
#include <vector>

//////////////////////////////
// Our data model
//////////////////////////////
namespace spatparse::csv
{
// Initialized: the parser fills these column by column, so a row that is missing one
// would otherwise read back whatever the previous row left on the stack.
struct xyz_position
{
  double x{}, y{}, z{};
};
struct aed_position
{
  double a{}, e{}, d{};
};

struct loudspeaker
{
  std::string name;
  std::variant<xyz_position, aed_position> position;
  double gain{1.0};
};

struct file
{
  std::vector<loudspeaker> speakers;
};
}
