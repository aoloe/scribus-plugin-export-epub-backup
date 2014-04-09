120812	concept for spans 2
120814	start implementing char formatting	3
120814	read and set char styles + first draft for char formatting stack	3
120815	char formatting as enum	1
120815	packaging for epub	1
120913	writing all metadata	2
120915	fix the compression for epub (mimetype!)	2
120915	export each section as an xhtml filex	 4
120916	css clean up + correctly appy css styles	4
120919	generate and generate the image for the cover	4
120926	refactor the class and file names	1
120926	test items out of page	1


the epub exporter does not allow you to design epub files.

it takes your SLA file (the scribus file) and exports all the content -- text with paragraph style, character styles, and some of the local formatting; images -- and packs it in a well formatted epub file.

you will then be able to open the epub file with sigil (or any other epub editor) and tweak it to your needs.

#Other possible formats
- Epub
  Versions 2.0.1 / 3.0
- Comic Book Archive file
  cbr (RAR); .cbz (ZIP); .cb7 (7z); .cbt (TAR); .cba (ACE)
- eReader
  .pdb
- iBook
  .ibooks
- Kindle
  .azw; .kf8
- Mobipocket
  .prc; .mobi
- Plucker
  .pdb

# Links

- http://www.idpf.org/epub/30/spec/epub30-publications.html#sec-core-media-types
- http://www.idpf.org/epub/30/spec/epub30-changes.html#sec-new-changed-audio-video

- http://www.mobileread.com/forums/showthread.php?p=2224241

- http://extensions.services.openoffice.org/en/project/Writer2ePub
- https://git.gnome.org/browse/libcroco/tree/
- https://launchpad.net/libcroco

- http://qt.gitorious.org/qt/qlalr
- http://blog.qt.digia.com/blog/2007/10/08/developer-dazetm-presents-a-closer-look-at-qlalr/
- http://qt-quarterly.developpez.com/qq-33/qlalr/
- http://www.qtcentre.org/threads/34726-QLALR-examples
- http://www.qtcentre.org/threads/30707-Parse-Text-File-with-Qt
- http://sourceforge.net/projects/htmlcxx/
- http://stackoverflow.com/questions/366028/what-is-a-good-c-c-css-parser
- http://htmlcxx.sourceforge.net/
- http://sourceforge.net/projects/csstidy/?source=dlp
- http://www.codeproject.com/Articles/20450/Simple-CSS-Parser
