/*
 * evaluateLog is a program to calculate some statistical properties based
 * on the data collected from BTSPP and put it into a form, that can
 * be pasted to tikzpicture for plotting.
 * Copyright (C) 2023 Jurek Rostalsky
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <algorithm>
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
constexpr std::string_view NUMBER_OF_EARS          = "ears";
constexpr std::string_view AVARAGE                 = "avg";
constexpr std::string_view VARIANCE                = "var";
constexpr std::string_view CORRELATION             = "corr";
constexpr std::string_view MAXIMUM                 = "max";
constexpr std::string_view QUANTILE                = "quan";
constexpr std::string_view RATIO                   = "ratio";
constexpr std::string_view BTSP                    = "btsp";
constexpr std::string_view BTSPP                   = "btspp";
constexpr std::string_view BTSVPP                  = "btsvpp";
constexpr std::string_view SPACE_SEPARATION        = "-space-separation";

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
  time,
  numberOfEars
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
    case Trait::numberOfEars :
      return numberOfEdgesInMinimallyBiconnectedGraph - numberOfNodes + 1;

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
  else if (str == NUMBER_OF_EARS) {
    return Trait::numberOfEars;
  }
  else {
    throw std::invalid_argument("Unknown trait <" + str + ">!");
  }
}

static void syntaxAdvice() {
  std::cout << "Syntax\n======\n";
  std::cout << "Type <./<programName> help> to see this page\n";
  std::cout << "./<programName> <filename> <problem type> <optional arg> <statistical property>:<trait>\n";
  std::cout << "The problem type can be either " << BTSP << " or " << BTSPP << " or " << BTSVPP << ".\n";
  std::cout << "If " << SPACE_SEPARATION << " is passed as <optional arg>, the ouput is separated by space and linebreak.\n";
  std::cout << "possible statistical properties: " << AVARAGE << ", " << VARIANCE << ", " << MAXIMUM << ", " << CORRELATION;
  std::cout << ", " << QUANTILE << std::endl;
  std::cout << "possible traits: " << NUMBER_OF_NODES << ", " << OBJECTIVE << ", " << LOWER_BOUND_ON_OPT << ", " << A_FORTIORI;
  std::cout << ", " << EDGE_COUNT << ", " << EDGE_COUNT_IN_MINIMALLY << ", " << TIME << ", " << NUMBER_OF_EARS << std::endl;
  std::cout << "examples for <statistical property>:<trait>\n";
  std::cout << AVARAGE << ":" << OBJECTIVE << std::endl;
  std::cout << MAXIMUM << ":" << RATIO << ":" << EDGE_COUNT << "," << EDGE_COUNT_IN_MINIMALLY << std::endl;
  std::cout << CORRELATION << ":" << RATIO << ":" << NUMBER_OF_NODES << "," << TIME << "," << A_FORTIORI << std::endl;
  std::cout << QUANTILE << ":0.95," << RATIO << ":" << EDGE_COUNT << "," << NUMBER_OF_NODES << std::endl;
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

static std::map<unsigned int, double> maximum(const std::map<unsigned int, std::vector<double>>& dataPoints) {
  std::map<unsigned int, double> maxes;
  for (const auto& dataPoint : dataPoints) {
    const std::vector<double>& vec = dataPoint.second;
    maxes.emplace(dataPoint.first, vec[std::distance(vec.begin(), std::max_element(vec.begin(), vec.end()))]);
  }
  return maxes;
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

static std::map<unsigned int, double> quantiles(const std::map<unsigned int, std::vector<double>>& dataPoints, double p) {
  p = std::min(p, 1.0);
  p = std::max(p, 0.0);
  std::map<unsigned int, double> quans;
  for (const auto& dataPoint : dataPoints) {
    std::vector<double> vec = dataPoint.second;
    std::sort(vec.begin(), vec.end());
    const double position   = p * (vec.size() - 1);
    const size_t left_index = std::floor(position);
    const double fraction   = position - left_index;
    if (left_index < vec.size() - 1) {
      quans.emplace(dataPoint.first, (1 - fraction) * vec[left_index] + fraction * vec[left_index + 1]);
    }
    else {
      quans.emplace(dataPoint.first, vec.back());
    }
  }
  return quans;
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
  if (!file) {
    throw std::runtime_error("Error: File <" + filename + "> not found!");
  }
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

static void writeToTerminal(const std::map<unsigned int, double>& dataPoints, const bool spaceSeparation) {
  if (spaceSeparation) {
    for (const auto& point : dataPoints) {
      std::cout << point.first << " " << point.second << std::endl;
    }
  }
  else {
    for (const auto& point : dataPoints) {
      std::cout << "(" << point.first << "," << point.second << ")";
    }
  }
  std::cout << std::endl << std::endl;
}

static void readArguments(int argc, const char** argv, const std::vector<Dataset>& data) {
  ProblemType type;
  bool spaceSeparation = false;
  for (int i = 2; i < argc; ++i) {
    std::string argument(argv[i]);
    if (argument == BTSP) {
      type = ProblemType::BTSP_approx;
    }
    else if (argument == BTSPP) {
      type = ProblemType::BTSPP_approx;
    }
    else if (argument == BTSVPP) {
      type = ProblemType::BTSVPP_approx;
    }
    else if (argument == SPACE_SEPARATION) {
      spaceSeparation = true;
    }
    else if (argument.starts_with(AVARAGE)) {
      argument = argument.substr(AVARAGE.length() + 1);
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> traits = splitInTwo(argument.substr(RATIO.length() + 1));
        writeToTerminal(avarages(extractRatioData(data, type, stringToTrait(traits[0]), stringToTrait(traits[1]))), spaceSeparation);
      }
      else {
        writeToTerminal(avarages(extractData(data, type, stringToTrait(argument))), spaceSeparation);
      }
    }
    else if (argument.starts_with(VARIANCE)) {
      argument = argument.substr(VARIANCE.length() + 1);
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> traits = splitInTwo(argument.substr(RATIO.length() + 1));
        writeToTerminal(variances(extractRatioData(data, type, stringToTrait(traits[0]), stringToTrait(traits[1]))), spaceSeparation);
      }
      else {
        writeToTerminal(variances(extractData(data, type, stringToTrait(argument))), spaceSeparation);
      }
    }
    else if (argument.starts_with(MAXIMUM)) {
      argument = argument.substr(MAXIMUM.length() + 1);
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> traits = splitInTwo(argument.substr(RATIO.length() + 1));
        writeToTerminal(maximum(extractRatioData(data, type, stringToTrait(traits[0]), stringToTrait(traits[1]))), spaceSeparation);
      }
      else {
        writeToTerminal(maximum(extractData(data, type, stringToTrait(argument))), spaceSeparation);
      }
    }
    else if (argument.starts_with(CORRELATION)) {
      static std::map<unsigned int, std::vector<double>> dataPoints1, dataPoints2;
      argument = argument.substr(CORRELATION.length() + 1);
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> strings  = splitInTwo(argument.substr(RATIO.length() + 1));
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
        std::array<std::string, 2> strings = splitInTwo(argument.substr(RATIO.length() + 1));
        dataPoints2                        = extractRatioData(data, type, stringToTrait(strings[0]), stringToTrait(strings[1]));
      }
      else {
        dataPoints2 = extractData(data, type, stringToTrait(argument));
      }
      writeToTerminal(correlations(dataPoints1, dataPoints2), spaceSeparation);
    }
    else if (argument.starts_with(QUANTILE)) {
      argument                           = argument.substr(QUANTILE.length() + 1);
      std::array<std::string, 2> strings = splitInTwo(argument);
      const double p                     = std::stod(strings[0]);
      argument                           = strings[1];
      if (argument.starts_with(RATIO)) {
        std::array<std::string, 2> traits = splitInTwo(argument.substr(RATIO.length() + 1));
        writeToTerminal(quantiles(extractRatioData(data, type, stringToTrait(traits[0]), stringToTrait(traits[1])), p), spaceSeparation);
      }
      else {
        writeToTerminal(quantiles(extractData(data, type, stringToTrait(argument)), p), spaceSeparation);
      }
    }
    else {
      throw std::invalid_argument("Unknown argument <" + argument + ">!");
    }
  }
}

int main(int argc, const char** argv) {
  if (std::string(argv[1]) == "help") {
    syntaxAdvice();
    exit(0);
  }
  std::vector<Dataset> data = castIntoDataFormat(parseFileIntoVector(argv[1]));
  readArguments(argc, argv, data);
  return 0;
}