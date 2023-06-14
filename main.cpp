#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

/*
 * Order:
 * instance type
 * number of nodes
 * objective
 * lower bound on opt
 * a fortiori guarantee
 * number of edges in biconnected graph
 * number of edges in minimally biconected graph
 * runtime
 */

constexpr std::string_view X_PREFIX        = "x:=";
constexpr std::string_view Y_PREFIX        = "y:=";
constexpr std::string_view NUMBER_OF_NODES = "nodes";
constexpr std::string_view A_FORTIORI      = "a-fortiori";
constexpr std::string_view AVARAGE         = "avg";
constexpr std::string_view CORRELATION     = "corr";

enum class ProblemType : unsigned int {
  BTSP_approx = 0,
  BTSPP_approx,
  BTSP_exact,
  BTSPP_exact,
  TSP_exact,
  NUMBER_OF_OPTIONS
};

enum class Trait : unsigned int {
  type = 0,
  numberOfNodes,
  objective,
  lowerBoundOnOpt,
  aFortioriGuarantee,
  numberOfEdgesInBiconnectedGraph,
  numberOfEdgesInMinimallyBiconnectedGraph,
  time
};

struct Dataset {
  ProblemType type;
  size_t numberOfNodes;
  double objective;
  double lowerBoundOnOpt;
  double aFortioriGuarantee;
  double numberOfEdgesInBiconnectedGraph;
  double numberOfEdgesInMinimallyBiconnectedGraph;
  double time;

  Dataset(const std::vector<double>& vec) :
    type(static_cast<ProblemType>(vec[0])),
    numberOfNodes(vec[1]),
    objective(vec[2]),
    lowerBoundOnOpt(vec[3]),
    aFortioriGuarantee(vec[4]),
    numberOfEdgesInBiconnectedGraph(vec[5]),
    numberOfEdgesInMinimallyBiconnectedGraph(vec[6]),
    time(vec[7]) {}

  double trait(const Trait trait) const {
    switch (trait) {
    case Trait::objective :
      return objective;
    case Trait::lowerBoundOnOpt :
      return lowerBoundOnOpt;
    case Trait::aFortioriGuarantee :
      return aFortioriGuarantee;
    case Trait::numberOfEdgesInBiconnectedGraph :
      return numberOfEdgesInBiconnectedGraph;
    case Trait::numberOfEdgesInMinimallyBiconnectedGraph :
      return numberOfEdgesInMinimallyBiconnectedGraph;
    case Trait::time :
      return time;

    default :
      throw std::invalid_argument("Unknown trait!");
      break;
    }
  }
};

class Measure {};

class Avarage : public Measure {
public:
  Trait x_trait;
  Trait avarage_trait;
};

class Correlation : public Measure {
public:
  Trait x_trait;
  Trait correlation_trait1;
  Trait correlation_trait2;
};

struct Properties {
  ProblemType type;
  Measure measure;
};

static std::map<unsigned int, std::vector<double>> extractData(const std::vector<Dataset>& data, const ProblemType& problemType, const Trait& trait) {
  std::map<unsigned int, std::vector<double>> dataPoints;
  for (const Dataset& set : data) {
    if (set.type == problemType) {
      dataPoints[set.numberOfNodes].push_back(set.trait(trait));
    }
  }
  return dataPoints;
}

static std::vector<double> avarages(const std::vector<Dataset>& data, const ProblemType& problemType, const Trait& trait) {
  std::map<unsigned int, std::vector<double>> dataPoints = extractData(data, problemType, trait);
  std::vector<double> avgs;
  avgs.reserve(dataPoints.size());
  for (const auto& dataPoint : dataPoints) {
    const std::vector<double>& vec = dataPoint.second;
    avgs.push_back(std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size());
  }
  return avgs;
}



static std::vector<double> splitLine(const std::string& line) {
  std::vector<double> splittedLine;
  std::stringstream lineStream(line);
  std::string cell;

  while (std::getline(lineStream, cell, ',')) {
    splittedLine.push_back(std::stod(cell));
  }
  return splittedLine;
}

static std::vector<std::vector<double>> parseFileIntoVector(const std::string& filename) {
  std::ifstream file(filename, std::ifstream::in);
  std::string line;
  std::vector<std::vector<double>> cells;
  while (std::getline(file, line)) {
    cells.emplace_back(splitLine(line));
  }
  return cells;
}

static std::vector<Dataset> castIntoDataFormat(const std::vector<std::vector<double>>& doubles) {
  std::vector<Dataset> data;
  data.reserve(doubles.size());
  for (const std::vector<double>& vec : doubles) {
    data.emplace_back(vec);
  }
  return data;
}

void writeToFile();

static void readArguments(int argc, const char** argv) {
  for (int i = 2; i < argc; ++i) {
    std::string argument(argv[i]);
    if (argument.starts_with(X_PREFIX)) {
      if (argument.substr(X_PREFIX.length(), NUMBER_OF_NODES.length()) == NUMBER_OF_NODES) {
        // do something
      }
    }
    else if (argument.starts_with(Y_PREFIX)) {
      if (argument.substr(Y_PREFIX.length(), AVARAGE.length()) == AVARAGE) {
        // do something else
      }
    }
  }
}

int main(int argc, const char** argv) {
  std::vector<Dataset> data = castIntoDataFormat(parseFileIntoVector(argv[1]));
  return 0;
}