#scribus-plugin-export-epub

THIS PROJECTS IS GETTING RENAMED AND CLOSED.

I'M BUILDING A NEW ONE FROM SCRATCH BY MOVING IN ONE SNIPPET AFTER THE OTHER.

Epub export plugin for Scribus

## Install

If no  `export/` directory exist in `scribus/plugins/` create it and

    ADD_SUBDIRECTORY(export)

in `scribus/plugins/CMakeList.txt` and create an empty `CMakeList.txt` in it.

Clone the `scribus-plugin-export-plugin` repository and put – or symlink – its `src` directory into `scribus/plugins/export/` as `epub`. Then add 

    ADD_SUBDIRECTORY(epub)

to `CMakeList.txt` file in `scribus/plugins/export/`.
