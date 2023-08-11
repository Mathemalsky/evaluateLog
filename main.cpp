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
constexpr std::string_view RATIO                   = "ratio";
constexpr std::string_view BTSP                    = "btsp";
constexpr std::string_view BTSPP                   = "btspp";
constexpr std::string_view BTSPP                   = "btsvpp";

enum class ProblemType : unsigned int {
  BTSP_approx = 0,
  BTSPP_approx,
  BTSVPP_approx,
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
    case Trait::numberOfNodes :
      return numberOfNodes;
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
  if (str == NUMBER_OF_NODES) {
    return Trait::numberOfNodes;
  }
  else if (str == OBJECTIVE) {
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
    throw std::invalid_argument("Unknown trait <" + str + ">!");
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

static std::map<unsigned int, std::vector<double>> extractRatioData(const std::vector<Dataset>& data,
                                                                    const ProblemType& problemType,
                                                                    const Trait& numerator,
                                                                    const Trait denominator) {
  std::map<unsigned int, std::vector<double>> dataPoints;
  for (const Dataset& set : data) {
    if (set.type == problemType) {
      dataPoints[set.numberOfNodes].push_back(set.trait(numerator) / set.trait(denominator));
    }
  }
  return dataPoints;
}

static std::map<unsigned int, double> avarages(const std::map<unsigned int, std::vector<double>>& dataPoints) {
  std::map<unsigned int, double> avgs;
  for (const auto& dataPoint : dataPoints) {
    const std::vector<double>& vec = dataPoint.second;
    avgs.emplace(dataPoint.first, std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size());
  }
  return avgs;
}

static std::map<unsigned int, double> variances(const std::map<unsigned int, std::vector<double>>& dataPoints) {
  const std::map<unsigned int, double> avgs = avarages(dataPoints);
  std::map<unsigned int, double> vars;
  for (const auto& dataPoint : dataPoints) {
    const std::vector<double>& vec = dataPoint.second;
    vars.emplace(dataPoint.first, std::accumulate(vec.begin(), vec.end(), 0.0, [&](double sum, double a) {
                                    return sum + std::pow(a - avgs.at(dataPoint.first), 2);
                                  }) / vec.size());
  }
  return vars;
}

static std::map<unsigned int, double> correlations(const std::map<unsigned int, std::vector<double>>& dataPoints1,
                                                   const std::map<unsigned int, std::vector<double>>& dataPoints2) {
  const std::map<unsigned int, double> avgs1 = avarages(dataPoints1);
  const std::map<unsigned int, double> avgs2 = avarages(dataPoints2);
  const std::map<unsigned int, double> vars1 = variances(dataPoints1);
  const std::map<unsigned int, double> vars2 = variances(dataPoints2);
  std::map<unsigned int, double> corrs;
  for (const auto& dataPoint : dataPoints1) {
    const double denominator       = std::sqrt(vars1.at(dataPoint.first) * vars2.at(dataPoint.first));
    const std::vector<double>& vec = dataPoint.second;
    corrs.emplace(
        dataPoint.first,
        std::inner_product(vec.begin(), vec.end(), dataPoints2.at(dataPoint.first).begin(), 0.0, std::plus<>(), [&](double a, double b) {
          return (a - avgs1.at(dataPoint.first)) * (b - avgs2.at(dataPoint.first));
        }) / (vec.size() * denominator));
  }
  return corrs;
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

static std::array<std::string, 2> splitInTwo(const std::string& str, const char delimiter = ',') {
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == delimiter) {
      return std::array<std::string, 2>{str.substr(0, i), str.substr(i + 1, str.length() - i - 1)};
    }
  }
  throw std::runtime_error("No delimiter " + std::to_string(delimiter) + " found in " + str);
}

static void writeToTerminal(const std::map<unsigned int, double>& dataPoints) {
  for (const auto& point : dataPoints) {
    std::cout << "(" << point.first << "," << point.second << ")";
  }
  std::cout << std::endl << std::endl;
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
      argument = argument.substr(AVARAGE.length() + 1, argument.length() - AVARAGE.length() - 1);
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> traits = splitInTwo(argument.substr(RATIO.length() + 1, argument.length() - RATIO.length() - 1));
        writeToTerminal(avarages(extractRatioData(data, type, stringToTrait(traits[0]), stringToTrait(traits[1]))));
      }
      else {
        writeToTerminal(avarages(extractData(data, type, stringToTrait(argument))));
      }
    }
    else if (argument.starts_with(VARIANCE)) {
      argument = argument.substr(VARIANCE.length() + 1, argument.length() - VARIANCE.length() - 1);
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> traits = splitInTwo(argument.substr(RATIO.length() + 1, argument.length() - RATIO.length() - 1));
        writeToTerminal(variances(extractRatioData(data, type, stringToTrait(traits[0]), stringToTrait(traits[1]))));
      }
      else {
        writeToTerminal(variances(extractData(data, type, stringToTrait(argument))));
      }
    }
    else if (argument.starts_with(CORRELATION)) {
      static std::map<unsigned int, std::vector<double>> dataPoints1, dataPoints2;
      argument = argument.substr(CORRELATION.length() + 1, argument.length() - CORRELATION.length() - 1);
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> strings  = splitInTwo(argument.substr(RATIO.length() + 1, argument.length() - RATIO.length() - 1));
        std::array<std::string, 2> strings2 = splitInTwo(strings[1]);
        Trait numerator                     = stringToTrait(strings[0]);
        Trait denominator                   = stringToTrait(strings2[0]);
        dataPoints1                         = extractRatioData(data, type, numerator, denominator);
        argument                            = strings2[1];
      }
      else {
        std::array<std::string, 2> traits = splitInTwo(argument);
        dataPoints1                       = extractData(data, type, stringToTrait(traits[0]));
        argument                          = traits[1];
      }
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> strings = splitInTwo(argument.substr(RATIO.length() + 1, argument.length() - RATIO.length() - 1));
        dataPoints2                        = extractRatioData(data, type, stringToTrait(strings[0]), stringToTrait(strings[1]));
      }
      else {
        dataPoints2 = extractData(data, type, stringToTrait(argument));
      }
      writeToTerminal(correlations(dataPoints1, dataPoints2));
    }
  }
}

int main(int argc, const char** argv) {
  std::vector<Dataset> data = castIntoDataFormat(parseFileIntoVector(argv[1]));
  readArguments(argc, argv, data);
  return 0;
}