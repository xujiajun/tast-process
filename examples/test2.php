<?php

include 'common.php';

function fun1() {
    sleep(10);
    echo "fun1 called".PHP_EOL;
}
function fun2() {
          sleep(10);
    echo "fun2 called".PHP_EOL;
}
function fun3() {
          sleep(10);
    echo "fun3 called".PHP_EOL;
}

$funcs = ['fun1','fun2','fun3'];
for ($i = 0; $i <= 2; ++$i) {
     // 创建子进程
    $pid = tast_fork();

    if ($pid == -1) {
        print "fork error".PHP_EOL;;
        exit(-1);
    }

    if (!$pid) {
        $mypid = tast_getpid();
        print "In child $i, pid $mypid ".PHP_EOL;;
        $j = $i+10;
        $funcs[$i]();

        exit($j);
    }
}

while (tast_waitpid(-1, $status,0) != -1) {
    $status = tast_wexitstatus($status);
    echo "Child $status completed".PHP_EOL;;
}