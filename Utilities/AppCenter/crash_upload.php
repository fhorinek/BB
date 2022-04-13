<?php

/*
error_reporting(E_ALL);

echo "request " . $_GET["req"] . "\n";

echo "\nget\n";
print_r($_GET);

echo "\npost\n";
print_r($_POST);

echo "\nfiles\n";
print_r($_FILES);
*/

if ($_FILES['file']['error'] == 0)
{
    $dump_file = tempnam("dumps", "");
    unlink($dump_file);
    $dump_file = $dump_file . ".zip";

    move_uploaded_file($_FILES['file']['tmp_name'], $dump_file);
    
    $secret = include "secret.php";
    
    $cmd = "/usr/bin/python3 scripts/upload_crash.py";
    $cmd .= " --secret=" . $secret;
    $cmd .= " --elf-search-path elf";
    $cmd .= " --crashdebug bin/CrashDebug";
    $cmd .= " --gdb bin/arm-none-eabi-gdb";
    $cmd .= " " . $dump_file;
    
    shell_exec("nohup " . $cmd . " > out.txt &");
}

?>
