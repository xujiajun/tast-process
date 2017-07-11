<?php

include 'common.php';

function fun1() {
    sleep(5);
    echo "fun1 called".PHP_EOL;
}
function fun2() {
          sleep(5);
    echo "fun2 called".PHP_EOL;
}
function fun3() {
          sleep(5);
    echo "fun3 called".PHP_EOL;
}

$funcs = ['fun1','fun2','fun3'];
$process_list = [];

for ($i = 0; $i <= 2; ++ $i) {
     // 创建子进程
    $pid = tast_fork();

    if ($pid == -1) {
        print "fork error".PHP_EOL;;
        exit(-1);
    }

    if (!$pid) {
         $j = $i+10;
        $funcs[$i]();
        $mypid = tast_getpid();
        print "In child $i, pid $mypid ".PHP_EOL;;
        exit($j);
    }

    if ($pid) {
        $process_list[] = $pid;
    }
}

if (!empty($process_list)) {
    for ($i = 0; $i < 2; ++ $i) {
        tast_waitpid($process_list[$i], $status, 0);
    }

    print("Children processes have exited.\n");
    print("Parent had exited.\n");
}
