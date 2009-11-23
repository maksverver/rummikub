<html>
<head>
 <title>GoT Programmeervuur Rummikub</title>
 <link rel="stylesheet" type="text/css" href="rummikub.css">
</head>

<body>
<form method="POST" action="play.php">
<table class="setup">
<tr><th></th><th>Naam</th><th>Request URL</th><th>Method</th></tr>
<?php for ($player = 0; $player < 4; ++$player) { ?>
<tr><th><?php echo ($player + 1) ?>:</th>
<td><input type="text" name="player_name[<?php echo $player ?>]" size="15"
    value="Simpel <?php echo ($player + 1)?>"></td>
<td><input type="text" name="player_url[<?php echo $player ?>]" size="60"
    value="http://hell.student.utwente.nl/rummikub/simple-player.php"></td>
<td><select name="player_method[<?php echo $player ?>]">
    <option value="GET">GET</option><option value="POST" selected>POST</option>
</td></tr>
<?php } ?>
<tr><td colspan="4" align="center">
<select name="style">
<option value="default" selected>Default Style</option>
<option value="alternate">Improved Readability</option>
</select>

<input type="submit" value="Start new game..."></td></tr>
</table>
</form>

</body>
</html>
