#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
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

constexpr std::string_view X_PREFIX                = "x:=";
constexpr std::string_view Y_PREFIX                = "y:=";
constexpr std::string_view NUMBER_OF_NODES         = "nodes";
constexpr std::string_view OBJECTIVE               = "objective";
constexpr std::string_view LOWER_BOUND_ON_OPT      = "lowerBoundOnOpt";
constexpr std::string_view A_FORTIORI              = "a-fortiori";
constexpr std::string_view EDGE_COUNT              = "edges";
constexpr std::string_view EDGE_COUNT_IN_MINIMALLY = "edgesInMinimally";
constexpr std::string_view TIME                    = "time";
constexpr std::string_view AVARAGE                 = "avg";
constexpr std::string_view VARIANCE                = "var";
constexpr std::string_view CORRELATION             = "corr";
constexpr std::string_view BTSP                    = "btsp";
constexpr std::string_view BTSPP                   = "btspp";

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

static Trait stringToTrait(const std::string& str) {
  if (str == OBJECTIVE) {
    return Trait::objective;
  }
  else if (str == LOWER_BOUND_ON_OPT) {
    return Trait::lowerBoundOnOpt;
  }
  else if (str == A_FORTIORI) {
    return Trait::aFortioriGuarantee;
  }
  else if (str == EDGE_COUNT) {
    return Trait::numberOfEdgesInBiconnectedGraph;
  }
  else if (str == EDGE_COUNT_IN_MINIMALLY) {
    return Trait::numberOfEdgesInMinimallyBiconnectedGraph;
  }
  else if (str == TIME) {
    return Trait::time;
  }
  else {
    throw std::invalid_argument("Unknown trait!");
  }
}

static std::map<unsigned int, std::vector<double>> extractData(const std::vector<Dataset>& data,
                                                               const ProblemType& problemType,
                                                               const Trait& trait) {
  std::map<unsigned int, std::vector<double>> dataPoints;
  for (const Dataset& set : data) {
    if (set.type == problemType) {
      dataPoints[set.numberOfNodes].push_back(set.trait(trait));
    }
  }
  return dataPoints;
}

static std::vector<std::pair<unsigned int, double>> avarages(const std::map<unsigned int, std::vector<double>>& dataPoints) {
  std::vector<std::pair<unsigned int, double>> avgs;
  avgs.reserve(dataPoints.size());
  for (const auto& dataPoint : dataPoints) {
    const std::vector<double>& vec = dataPoint.second;
    avgs.emplace_back(dataPoint.first, std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size());
  }
  return avgs;
}

static std::vector<std::pair<unsigned int, double>> variances(const std::map<unsigned int, std::vector<double>>& dataPoints) {
  const std::vector<std::pair<unsigned int, double>> avgs = avarages(dataPoints);
  std::vector<std::pair<unsigned int, double>> vars;
  vars.reserve(dataPoints.size());
  for (const auto& dataPoint : dataPoints) {
    const std::vector<double>& vec = dataPoint.second;
    vars.emplace_back(dataPoint.first, std::accumulate(vec.begin(), vec.end(), 0.0, [&](double sum, double a) {
                                         return sum + std::pow(a - avgs[dataPoint.first].second, 2);
                                       }) / vec.size());
  }
  return vars;
}

// static std::vector<std::pair<unsigned int, double>> correlations(const std::map<unsigned int, std::vector<double>>& dataPoints,
//                                                                  const Trait& trait1,
//                                                                  const Trait& trait2) {
//   const std::vector<std::pair<unsigned int, double>> avgs1 = avarages(dataPoints, problemType, trait1);
//   const std::vector<std::pair<unsigned int, double>> avgs2 = avarages(dataPoints, problemType, trait2);
//   const std::vector<std::pair<unsigned int, double>> vars1 = variances(dataPoints, problemType, trait1);
//   const std::vector<std::pair<unsigned int, double>> vars2 = variances(dataPoints, problemType, trait2);
//   std::vector<std::pair<unsigned int, double>> corrs;
//   corrs.reserve(dataPoints.size());
//   for (const auto& dataPoint : dataPoints) {
//     const double denominator = std::sqrt(vars1[dataPoint.first] * vars2[dataPoint.first]);
//     const std::vector<double>& vec = dataPoint.second;
//     corrs.emplace_back(dataPoint.first, std::accumulate(vec.begin(), vec.end(), 0, [&](double sum, )))
//   }
// }

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

void writeToTerminal(const std::vector<std::pair<unsigned int, double>>& dataPoints) {
  for (const auto& point : dataPoints) {
    std::cout << "(" << point.first << "," << point.second << ")";
  }
  std::cout << std::endl;
}

static void readArguments(int argc, const char** argv, const std::vector<Dataset>& data) {
  ProblemType type;
  for (int i = 2; i < argc; ++i) {
    std::string argument(argv[i]);
    if (argument == BTSP) {
      type = ProblemType::BTSP_approx;
    }
    else if (argument == BTSPP) {
      type = ProblemType::BTSPP_approx;
    }
    else if (argument.starts_with(AVARAGE)) {
      Trait trait = stringToTrait(argument.substr(AVARAGE.length() + 1, argument.length() - AVARAGE.length() - 2));
      std::map<unsigned int, std::vector<double>> dataPoints = extractData(data, type, trait);
      writeToTerminal(avarages(dataPoints));
    }
    else if (argument.starts_with(VARIANCE)) {
      Trait trait = stringToTrait(argument.substr(VARIANCE.length() + 1, argument.length() - VARIANCE.length() - 2));
      std::map<unsigned int, std::vector<double>> dataPoints = extractData(data, type, trait);
      writeToTerminal(variances(dataPoints));
    }
    else if (argument.starts_with(CORRELATION)) {
      std::stringstream traitsString(argument.substr(CORRELATION.length() + 1, argument.length() - CORRELATION.length() - 3));
      std::string traitString;
      std::getline(traitsString, traitString, ',');
      Trait trait1 = stringToTrait(traitString);
      std::getline(traitsString, traitString, ',');
      Trait trait2 = stringToTrait(traitString);
    }
  }
}

int main(int argc, const char** argv) {
  std::vector<Dataset> data = castIntoDataFormat(parseFileIntoVector(argv[1]));
  readArguments(argc, argv, data);
  return 0;
}