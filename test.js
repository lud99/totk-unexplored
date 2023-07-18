let m_ButtonsPerPage = 11;
let m_NumberOfPages = 2;

let buttonTypes = [
    "Koroks",
    "Shrines",
    "Lightroots",
    "Bubbuls",
    
    "Caves",
    "Wells",
    "Chasms",
    "Locations",

    "Hinoxes",
    "Taluses",
    "Moldugas",
    "FluxConstructs",
    "Froxes",
    "Gleeoks",

    "SagesWills",
    "OldMaps",
    "AddisonSigns",
    "SchemaStones",
    "YigaSchematics",
    "ShowCompleted",
    "Count"
]

for (var j = 0; j < m_NumberOfPages; j++)
{

    let end = Math.min(20, (j + 1) * m_ButtonsPerPage);
    for (var i = j * m_ButtonsPerPage - (1 * j); i < end; i++)
    {
        let b = buttonTypes[i];
        if (i == end - 1) b = "ShowCompleted";

        console.log(j, i, b);
        //console.log((i + j) % (m_ButtonsPerPage) )
    }
}