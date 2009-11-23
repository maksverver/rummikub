<?php
require_once('game.inc.php');
error_reporting(E_ALL);

$player_name   = @$_REQUEST['player_name'];
$player_url    = @$_REQUEST['player_url'];
$player_method = @$_REQUEST['player_method'];
$player_tiles  = @$_REQUEST['player_tiles'];
$table_tiles   = @$_REQUEST['table_tiles'];
$pool_tiles    = @$_REQUEST['pool_tiles'];
$next_player   = intval(@$_REQUEST['next_player']);
$pass_count    = intval(@$_REQUEST['pass_count']);
$style         = @$_REQUEST['style'];

if (!empty($player_tiles))
{
    foreach($player_tiles as $key => $value)
        $player_tiles[$key] = parse_tiles($value);
}

$table_tiles = empty($table_tiles) ? array() : parse_table($table_tiles);
$pool_tiles  = empty($pool_tiles) ? array() : parse_tiles($pool_tiles);

if (empty($player_tiles) || ($next_player < 0 || $next_player >= 4))
{
    // Initialize new game
    $pool = generate_pool();
    $player_tiles = array( array_slice($pool,  0, 14),
                           array_slice($pool, 14, 14),
                           array_slice($pool, 28, 14),
                           array_slice($pool, 42, 14) );
    $pool_tiles = array_slice($pool, 56);
    $table_tiles = array();
    $next_player = 0;
    $new_game = true;
}
else
{
    $new_game = false;
}

if (empty($player_name)    || count($player_name)   != 4 ||
    empty($player_url)     || count($player_url)    != 4 ||
    empty($player_method)  || count($player_method) != 4 ||
    empty($player_tiles)   || count($player_tiles)  != 4)
{
    die("Invalid game state!");
}

$styles = array(
    'default'   => 'Default Style',
    'alternate' => 'Improved Readability' );
if (!array_key_exists($style, $styles)) $style = 'default';

function is_game_over()
{
    global $pool_tiles, $player_tiles, $pass_count;
    for ($player = 0; $player < 4; ++$player)
    {
        if (count($player_tiles[$player]) == 0)
            return true;
    }
    if ($pass_count == 4)
        return true;
    return false;
}

function echo_tile($tile)
{
/*
    echo '<div class="tile', substr("RGBK", color_of($tile), 1), '">', value_of($tile), '</div>';
*/
    $c = substr("RGBK", color_of($tile), 1);
    echo '<div class="tile', $c, '"><span class="color">', $c, '</span>', value_of($tile), '</div>';
}

function echo_game_state()
{
    global $player_name, $player_tiles, $table_tiles, $pool_tiles, $next_player;
?><table class="gamestate"><tr><td>
    <table class="players"><?php
    for ($player = 0; $player < 4; ++$player)
    {
        echo ($player == $next_player) ? '<tr class="currentPlayer">' : '<tr>';
        echo '<th>', htmlentities($player_name[$player]), '<div class="score">', -count_score($player_tiles[$player]), '&nbsp;punten</div></th>';
        echo '<td><div class="rack">';
        foreach ($player_tiles[$player] as $tile) echo_tile($tile);
        echo '</div></td>';
        echo '</tr>';
    }
    ?></table>
</td><td>
    <div class="table" style="height:100%">
    <?php
    foreach($table_tiles as $tiles)
    {
        echo '<div class="set">';
        foreach($tiles as $tile) echo_tile($tile);
        echo '</div>';
    }
    ?>
    </div>
    <p style="text-align: center"><?php echo count($pool_tiles); ?> stenen over</p>
</td></tr></table><?php
}

?>
<html>
<head>
 <title>GoT Programmeervuur Rummikub</title>
 <link rel="stylesheet" type="text/css" href="rummikub.css">
<?php foreach ($styles as $id => $name) { ?><link type="text/css" 
    rel="<?php echo ($id == $style ? "stylesheet" : "alternate stylesheet") ?>"
    href="rummikub-<?php echo $id ?>.css"
    title="<?php echo htmlentities($name); ?>">
<?php } ?>
</head>

<body>

<?php
$message = '';
if (!$new_game && !is_game_over())
{
    echo '<h2>Vorige situatie</h2>';
    echo_game_state();

    // Build query string:
    $opponents_tiles = array();
    for ($i = 1; $i < 4; ++$i)
        $opponents_tiles[] = count($player_tiles[($next_player + $i)%4]);
    $query =
        'yourTiles='.encode_tiles($player_tiles[$next_player]).'&'.
        'table='.encode_table($table_tiles).'&'.
        'poolTiles='.count($pool_tiles).'&'.
        'opponentsTiles='.implode('.', $opponents_tiles);

    // Build cURL request:
    $post = ($player_method[$next_player] == 'POST');
    $url = $player_url[$next_player];
    $ch = curl_init($post ? $url : $url.'?'.$query);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER , TRUE);
    curl_setopt($ch, CURLOPT_HEADER, FALSE);
    if ($post)
    {
        curl_setopt($ch, CURLOPT_POST, TRUE);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $query);
    }

    // Execute request:
    $data = curl_exec($ch);

    // Parse response
    $played = FALSE;
    $taken  = FALSE;
    if ($data === FALSE)
    {
        $message = 'cURL request faalde.';
    }
    else
    {
        $data = trim($data);
        if (empty($data))
            $message = 'Responsdata is leeg.';
        else
        if ($data != 'draw')
        {
            // Note: parsing also verifies sets are valid
            $new_table = parse_table($data);
            if (!$new_table)
            {
                $message = 'Ongeldige tafelomschrijving ontvangen.';
            }
            else
            {
                // Verify that the new board can be made using tiles on the
                // table and on the player's rack:
                $a = array();  // tiles on table
                foreach ($table_tiles as $tiles)
                    $a = array_merge($a, $tiles);
                $b = $player_tiles[$next_player];  // tiles on rack
                $c = array();  // tiles played
                $valid = true;
                foreach ($new_table as $tiles)
                {
                    foreach ($tiles as $tile)
                    {
                        if (remove_one($a, $tile)) {
                            // taken from table
                        }
                        else
                        if (remove_one($b, $tile)) {
                            $c[] = $tile;  // taken from rack
                        } else {
                            $message = 'Speler gebruikt een steen die niet beschikbaar is: '.encode_tile($tile).'.';
                            $valid = false;
                            break;
                        }
                    }
                    if (!$valid) break;
                }

                if ($valid && count($a) > 0)
                {
                    $message = 'Speler heeft niet alle stenen op tafel gebruikt.';
                    $valid = false;
                }

                if ($valid && count($b) == count($player_tiles[$next_player]))
                {
                    $message = 'Speler heeft geen nieuwe stenen op tafel geplaatst.';
                    $valid = false;
                }

                if ($valid)
                {
                    // Valid move!
                    $played = $c;
                    $table_tiles = $new_table;
                    $player_tiles[$next_player] = $b;
                    $pass_count = 0;
                }
            }
        }
    }

    if (!$played)
    {
        // Have not made a valid move yet
        if (count($pool_tiles) > 0)
        {
            // Draw new tile instead:
            $taken = array_shift($pool_tiles);
            $player_tiles[$next_player][] = $taken;
            $pass_count = 0;
        }
        else
        {
            // Must pass!
            ++$pass_count;
        }
    }

    $previous_player = $next_player;
    $next_player = ($next_player + 1)%4;
}

if (isset($previous_player))
{
    echo '<p>', htmlentities($player_name[$previous_player]);
    if (!empty($message))
    {
        echo ' kon niet zetten. ', $message;
        if (!empty($data))
            echo ' Respons was: <pre>', htmlentities($data), '</pre>';
        echo '</p>';
    }
    else
    if ($played !== FALSE)
    {
        echo ' speelt: ';
        foreach ($played as $tile) echo_tile($tile);
    }
    else
    if ($taken !== FALSE)
    {
        echo ' trekt: '; echo_tile($taken);
    }
    else
    {
        echo ' past.';
    }
    echo '</p>';
}

echo '<h2>Huidige situatie</h2>';
echo_game_state();
?>

<form method="POST" action="play.php">
<?php
for ($player = 0; $player < 4; ++$player)
{
    echo '<input type="hidden" name="player_name[', $player, ']" value="',
        htmlspecialchars($player_name[$player]), '">';
    echo '<input type="hidden" name="player_url[', $player, ']" value="',
        htmlspecialchars($player_url[$player]), '">';
    echo '<input type="hidden" name="player_method[', $player, ']" value="',
        htmlspecialchars($player_method[$player]), '">';
    echo '<input type="hidden" name="player_tiles[', $player, ']" value="',
        htmlspecialchars(encode_tiles($player_tiles[$player])), '">';
}
echo '<input type="hidden" name="table_tiles" value="',
    htmlspecialchars(encode_table($table_tiles)), '">';
echo '<input type="hidden" name="pool_tiles" value="',
    htmlspecialchars(encode_tiles($pool_tiles)), '">';
echo '<input type="hidden" name="next_player" value="',
    htmlspecialchars($next_player), '">';
echo '<input type="hidden" name="pass_count" value="',
    htmlspecialchars($pass_count), '">';
echo '<input type="hidden" name="style" value="', htmlspecialchars($style), '">';
if (!is_game_over()) echo '<p><input type="submit" value="Doorgaan..."></p>';
?>
</form>
</body>
</html>
