Set-Content -Path ".\hello.odin" -Value "package main;

import `"core:fmt`";

main :: proc(){`n`tfmt.printf(`"Hello, world! Your Odin project is set up.\n`")`n};" -Force
Set-Content -Path "compile-and-run.ps1" -Value "odin build . -out:hello.exe -debug `r`n.\hello.exe" -Force
Set-Content -Path "ols.json" -Value "{
    `"collections`": [
      {
        `"name`": `"core`",
        `"path`": `"C:\\Users\\krisd\\Documents\\Programming\\Odin\\Odin\\core`"
      }
    ],
    `"enable_document_symbols`": true,
    `"enable_semantic_tokens`": true,
    `"enable_hover`": true,
    `"enable_snippets`": true
  }
  "
.\compile-and-run.ps1