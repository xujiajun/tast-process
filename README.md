# tast-process
An extension providing multi process for PHP 7

## Requirements

- PHP7.0+


## Installation

```
git clone https://github.com/xujiajun/tast-process.git
cd tast-process
phpize
./configure
make && make install
```

### Quick Start

```
<?php

if (!extension_loaded('tast_process')) {
    echo "tast_process extension is not enabled\n";
    exit(1);
}

function func1()
{
    echo "call func1.start".PHP_EOL;
    sleep(5);
    echo "call func1.end".PHP_EOL;
}
function func2()
{
    echo "call func2.start".PHP_EOL;
    sleep(5);
    echo "call func2.end".PHP_EOL;
}
function func3()
{
    echo "call func3.start".PHP_EOL;
    sleep(5);
    echo "call func3.end".PHP_EOL;
}

// multi process 
tast_process(['func1','func2','func3'],3);
```


## License
[MIT Licensed](http://www.opensource.org/licenses/MIT)
