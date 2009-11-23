<?php

error_reporting(E_ALL);
include('game.inc.php');

header('Content-Type: text/plain');

// Grab arguments
$my_tiles        = parse_tiles(@$_REQUEST['yourTiles']);
$table           = parse_table(@$_REQUEST['table']);
$pool_tiles      = intval(@$_REQUEST['poolTiles']);
$opponents_tiles = parse_ints(@$_REQUEST['opponentsTiles'], '.');

// Check validity of arguments (even those we don't use; useful for debugging)
if ($my_tiles === FALSE)
    die('Invalid tiles for me');
if ($table === FALSE)
    die('Invalid table tiles');
if (count($my_tiles) == 0)
    die('I have no tiles left!');
if ($pool_tiles < 0)
    die('Too few pool tiles left!');
if (count($opponents_tiles) != 3)
    die('Expected three opponents!');
for ($i = 0; $i < 3; ++$i) {
    if ($opponents_tiles[$i] <= 0)
        die('Opponent '.($i+1).' has too few tiles left!');
}
$total_tiles = $pool_tiles + count($my_tiles);
for ($i = 0; $i < 3; ++$i)
    $total_tiles += $opponents_tiles[$i];
for ($i = 0; $i < count($table); ++$i)
    $total_tiles += count($table[$i]);
if ($total_tiles != 104)
    die("Total number of tiles should be 104 (not $total_tiles)!");

// Find a valid set to play:
$new_tables = array();
for ($k = 0; $k < count($my_tiles); ++$k)
{
    for ($j = 0; $j < $k; ++$j)
    {
        for ($i = 0; $i < $j; ++$i)
        {
            $new_set = array($my_tiles[$i], $my_tiles[$j], $my_tiles[$k]);
            sort($new_set);
            if (valid_set($new_set))
            {
                $new_table = $table;
                $new_table[] = $new_set;
                $new_tables[] = $new_table;
            }
        }
    }
}

// Find a single tile to add to an existing group:
for ($i = 0; $i < count($my_tiles); ++$i)
{
    for ($j = 0; $j < count($table); ++$j)
    {
        $new_set = $table[$j];
        $new_set[] = $my_tiles[$i];
        sort($new_set);
        if (valid_set($new_set))
        {
            $new_table = $table;
            $new_table[$j] = $new_set;
            $new_tables[] = $new_table;
        }
        else
        {
            // Try to split:
            for ($k = 1; $k < count($new_set); ++$k)
            {
                $left  = array_slice($new_set, 0, $k);
                $right = array_slice($new_set, $k);
                if (valid_set($left) && valid_set($right))
                {
                    $new_table = $table;
                    array_splice($new_table, $j, 1, array($left, $right));
                    $new_tables[] = $new_table;
                }
            }
        }
    }
}

// See if I can make a group by taking two tiles from the rack and one from the board:
for ($j = 0; $j < count($my_tiles); ++$j)
{
    for ($i = 0; $i < $j; ++$i)
    {
        for ($k = 0; $k < count($table); ++$k)
        {
            if (count($table[$k]) >= 4)
            {
                for ($l = 0; $l < count($table[$k]); ++$l)
                {
                    $a = array($my_tiles[$i], $my_tiles[$j], $table[$k][$l]);
                    sort($a);
                    $b = $table[$k];
                    unset($b[$l]);
                    sort($b);
                    if (valid_set($a) && valid_set($b))
                    {
                        $new_table = $table;
                        $new_table[$k] = $b;
                        $new_table[] = $a;
                        $new_tables[] = array_values($new_table);
                    }
                }
            }
        }
    }
}

if (count($new_tables) == 0)
{
    // No moves availabe; draw a new tile
    echo "draw\n";
}
else
{
    // Pick a random move
    $new_table = $new_tables[rand(0, count($new_tables) - 1)];
    echo encode_table($new_table), "\n";
}
?>
