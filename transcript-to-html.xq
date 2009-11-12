declare variable $file external;
declare variable $doc := doc($file)/rummikub-transcript;

declare function local:format-tiles($tiles as xs:string) {
  for $tile in tokenize($tiles, "[.]")
  return
    <div class="tile{substring($tile,1,1)}">{substring($tile,2)}</div>
};

declare function local:format-table($table as xs:string) {
  <div class="table">{
    for $set in tokenize($table, "[-]")
    return <div class="set">{local:format-tiles($set)}</div>
  }</div>
};

declare function local:remove-tile($tiles as xs:string, $tile as xs:string) {
    let $tiles := tokenize($tiles, '[.]')
    return string-join(remove($tiles, index-of($tiles, $tile)[1]), '.')
};

declare variable $nav-bar :=
  <ul class="nav-bar">
    <li><a href="#config">Initial Configuration</a></li>
    <li><a href="#transcript">Game Transcript</a></li>
    <li><a href="#scores">Final Scores</a></li>
  </ul>;

<html>
<head>
 <title>Rummikub Game Report</title>
 <link rel="stylesheet" type="text/css" href="transcript.css" />
 </head>
 <body>
  <h1>Rummikub Game Report</h1>
  {$nav-bar}

  <h2 id="setup">Initial Configuration</h2>
  <h3>Players</h3>
  <ol class="players">
  {
    for $player in $doc/setup/players/player return 
    <li>
     <div class="name">{$player/name/text()}</div>
     <div class="rack">{local:format-tiles($player/tiles/text())}</div>
    </li>
  }
  </ol>
  <h3>Pool tiles:</h3>
  <div class="pool">{local:format-tiles($doc/setup/pool)}</div>

  <h2 id="transcript">Game Transcript</h2>
  <ol class="turns">
  {
    for $turn in $doc/turn
    let $name := $doc/setup/players/player[@id=$turn/@player]/name/text()
    let $tiles := $turn/tiles/text()

    return
        <li class="turn"><div class="name">{$name}</div>
        { for $msg in $turn/error return
          <div class="error">Failure: {$msg/text()}!</div> }
        { for $played in $turn/played return
          (<div class="rack">{local:format-tiles(string($tiles))}
              <img src="ra.png" style="vertical-align: middle" alt="-&gt;"/>
              {local:format-tiles($played/text())}
           </div>, local:format-table($turn/table/text())) }
        { for $drawn in $turn/drawn
          let $prev-tiles := local:remove-tile($tiles, $drawn/text())
          return
          (<div class="rack">{local:format-tiles($prev-tiles)}
             <img src="la.png" style="vertical-align: middle" alt="&lt;-"/>
                {local:format-tiles($drawn/text())}
            </div>,
            <p>{data($turn/pool/@size)} tiles left.</p>) }
        </li>
  }
  </ol>

  <h2 id="scores">Final Scores</h2>
  <ol class="scores">{
    for $score in $doc/scores/score return
    <li class="score">
     <div class="name">{$score/player/text()}</div>
     <div class="score">{$score/value/text()}</div>
    </li>
  }</ol>

 </body>
</html>
