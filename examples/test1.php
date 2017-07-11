<?php

include 'common.php';

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
// $re = tast_kill(13038,SIGKILL);
// var_dump($re);
// tast_process(['func1',123,'func3'],3);
tast_process(['func1','func2'],2);