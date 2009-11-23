<?php

function parse_ints($x, $sep)
{
    if (!$x) return array();
    $x = explode($sep, $x);
    for ($i = 0; $i < count($x); ++$i)
        $x[$i] = intval($x[$i]);
    return $x;
}

function parse_tile($x)
{
    if (strlen($x) > 1)
    {
        if (($color = strpos("RGBK", $x{0})) !== FALSE)
        {
            $value = intval(substr($x, 1));
            if ($value >= 1 && $value <= 13)
                return make_tile($color, $value);
        }
    }
    return FALSE;
    //die("Couldn't parse tile '$x'!");
}

function parse_tiles($x)
{
    if (!$x) return array();
    $x = explode('.', $x);
    for ($i = 0; $i < count($x); ++$i)
        $x[$i] = parse_tile($x[$i]);
    return $x;
}

function parse_table($x)
{
    if (!$x) return array();
    $x = explode('-', $x);
    for ($i = 0; $i < count($x); ++$i)
    {
        $set = parse_tiles($x[$i]);
        sort($set);
        if (!valid_set($set)) return FALSE;
        // die("Tiles $x[$i] do not form a valid set!");
        $x[$i] = $set;
    }
    return $x;
}

function encode_tile($tile)
{
    return substr('RGBK', color_of($tile), 1).value_of($tile);
}

function encode_tiles($tiles)
{
    return implode('.', array_map('encode_tile', $tiles));
}

function encode_table($table)
{
    return implode('-', array_map('encode_tiles', $table));
}

function count_score($tiles)
{
    $score = 0;
    foreach ($tiles as $tile)
        $score += value_of($tile);
    return $score;
}

function make_tile($color, $value)
{
    if ($color < 0 || $color > 3 || $value < 1 || $value > 13)
        die("invalid tile: $color/$value");
    return 13*$color + ($value - 1);
}

function color_of($tile)
{
    return ($tile - $tile%13)/13;
}

function value_of($tile)
{
    return $tile%13 + 1;
}

function is_run($tiles)
{
    $color = color_of($tiles[0]);
    $value = value_of($tiles[0]);
    foreach ($tiles as $tile)
    {
        if (color_of($tile) != $color || value_of($tile) != $value)
            return false;
        ++$value;
    }
    return true;
}

function is_group($tiles)
{
    $value = value_of($tiles[0]);
    $colors_used = 0;
    foreach ($tiles as $tile)
    {
        if (value_of($tile) != $value) return false;
        $color_mask = 1<<color_of($tile);
        if ($colors_used & $color_mask) return false;
        $colors_used |= $color_mask;
    }
    return true;
}

function valid_set($tiles)
{
    return count($tiles) >= 3 && (is_run($tiles) || is_group($tiles));
}

function generate_pool()
{
    $pool = array();
    for ($color = 0; $color < 4; ++$color)
    {
        for ($value = 1; $value <= 13; ++$value)
        {
            for ($count = 0; $count < 2; ++$count)
                $pool[] = make_tile($color, $value);
        }
    }
    shuffle($pool);
    return $pool;
}

function remove_one(&$arr, $tile)
{
    $i = array_search($tile, $arr);
    if ($i === FALSE) return FALSE;
    unset($arr[$i]);
    $arr = array_values($arr);
    return TRUE;
}

?>