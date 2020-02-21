![](https://github.com/demiand/node-matrix-profile/workflows/Main%20workflow/badge.svg?branch=master)

# node-matrix-profile

node-matrix-profile provides a binding to use [SCAMP](https://github.com/zpzim/SCAMP) in your Node.js applications. Note that JavaScript values are converted to C/C++ values and vice-versa, which cause a decrease in runtime.

## Install

```bash
npm install --save node-matrix-profile
```

## How to use

```js
const mp = require('node-matrix-profile');

const result = mp.calculate({
  window_size: 3,
  timeseries_a: data
});

console.log(result);
```

## Limitations

- Only profile type `1NN_INDEX` is currently supported.

## Development

```bash
git clone git@github.com:DemianD/node-matrix-profile.git
npm install
```
