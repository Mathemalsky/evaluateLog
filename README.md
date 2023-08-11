# Evaluate log

## Syntax
`./evaluateLog <filename> <problem type> <statistical property>:<trait>`

The problem type must be either `btsp` or `btspp`.
The statistical property can be chosen out of:
- mean (`avg`)
- variance (`var`)
- maximum (`max`)
- correlation coefficient (`corr`)
- quantile (`quan`)

The traits can be chosen from:
- number of nodes (`nodes`)
- objective value (`objective`)
- lower bound on optimum (`lowerBoundOnOpt`)
- a fortiori guarantee (`a-fortiori`)
- edges in bottleneck optimal biconnected subgraph (`edges`)
- edges in minimally biconnected subgraph (`edgesInMinimally`)
- runtime (`time`)

A trait can be replaced by `ratio:<trait1>,<trait2>` to process a ratio of traits instead. The correlation coefficient requires two traits separated by a comma. To compute a quantile the p-value must be passed first, followed by a comma and the trait. It is possible to append further combinations `<statistical property>:<trait>` to the command. Then all those combinations are calculated and output in the order of the arguments.

### Examples
```
./evaluateLog data.csv btsp corr:edges,edgesInMinimally
./evaluateLog data.csv btsp quan:0.95,edgesInMinimally
./evaluateLog data.csv btspp avg:ratio:edges,edgesInMinimally
```