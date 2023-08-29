# Evaluate log
This little program extracts the log data collected from running [BTSPP](https://github.com/Mathemalsky/BTSPP) and calculates some statistical properties. Afterwards, the calculated features are formatted in a way that can be put into tikzpictures.

## Syntax
`./evaluateLog <filename> <problem type> <optionl arg> <statistical property>:<trait>`

The problem type must be either `btsp` or `btspp`.
The argument `-space-separation` can be passed as optional argument to print each pair of values in its own line separated by a space character, instead of the bracket notation.

The statistical property can be chosen out of:
- mean (`avg`)
- variance (`var`)
- standard deviation (`dev`)
- maximum (`max`)
- correlation coefficient (`corr`)
- quantile (`quan`)

The traits can be chosen from:
- number of nodes (`nodes`)
- objective value (`objective`)
- lower bound on optimum (`lowerBoundOnOpt`)
- a fortiori guarantee (`a-fortiori`)
- number edges in bottleneck optimal biconnected subgraph (`edges`)
- number edges in minimally biconnected subgraph (`edgesInMinimally`)
- runtime (`time`)
- number of ears in the open ear decomposition of the minimally biconnected subgraph (`ears`)

A trait can be replaced by `ratio:<trait1>,<trait2>` to process a ratio of traits instead. The correlation coefficient requires two traits separated by a comma. To compute a quantile the p-value must be passed first, followed by a comma and the trait. It is possible to append further combinations `<statistical property>:<trait>` to the command. Then all those combinations are calculated and output in the order of the arguments.

### Examples
```
./evaluateLog data.csv btsp corr:edges,edgesInMinimally
./evaluateLog data.csv btsp quan:0.95,edgesInMinimally
./evaluateLog data.csv btspp avg:ratio:edges,edgesInMinimally
```