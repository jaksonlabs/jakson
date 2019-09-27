# Binary Strings

Carbon Type     | Description                       | Size                      | *null*-Value | Marker 
----------------|-----------------------------------|---------------------------|--------------|--------
`binary`        | binary of `n` bytes (MIME typed)  | `n` + 2<sup>+</sup> bytes | `null`       | `[b]`
`custom binary` | binary of `n` bytes (custom type) | `n` + 3<sup>+</sup> bytes | `null`       | `[x]`


## Encoding As Field Value

### With Known MIME Types

```
[b](mime-type-id)(str-len) <binary>
```

Description          | Size (in Byte)      | Marker         | Payload
---------------------|---------------------|----------------|-------------------------------------------------
 binary of `n` bytes | 1 + `k` + `l` + `n` | `[b]` (`binary`) | *see below*

Payload			   | Description                     | Encoding Type             | Size
-------------------|---------------------------------|---------------------------|---------
`(mime-type-id)`   | MIME Type Id (see below)        | variable-length integer   | `k` byte
`(str-len)`        | binary string_buffer length in bytes   | variable-length integer   | `l` byte
 `<binary>` | the binary string_buffer itself        | fixed-sized binary string_buffer | `n` byte

## Example

snippet
```
MIME Type:	text/html
Content:	<html><body><p>Hello, World!</p></body></html>
```

A (compacted) Carbon file, which encodes the HTML code as `binary`.

```
[b](219)(46) [<html><body><p>Hello, World!</p></body></html>]
```

### With Unknown MIME Types

```
[x](type-str-len)<type-str>(str-len) <binary-string_buffer>
```

 Description          | Size (in Byte)            | Marker         | Payload
----------------------|---------------------------|----------------|-------------
 binary of `n` bytes  | 1 + `p` + `q` + `l` + `n` | `[x]` (`custom binary`) | *see below*

Payload			         | Description                             | Encoding Type                | Size
-------------------------|-----------------------------------------|------------------------------|---------
`(type-str-len)` | used-def type string_buffer length | variable-length integer      | `k` byte
`<type-str>`     | the user-def type string_buffer itself     | fixed-sized string_buffer | `q` byte
`(str-len)`              | binary string_buffer length in bytes           | variable-length integer      | `l` byte
 `<binary>`       | the binary string_buffer itself                | fixed-sized binary string_buffer    | `n` byte
 

## Example

snippet
```
Type Name:      custom-type
Content:        c3RpbGwgYSBzZWNyZXQ=
```

A (compacted) Carbon file, which encodes the HTML code as `binary`.

```
[x](11)[custom-type](20) [c3RpbGwgYSBzZWNyZXQ=]
```

### Parsing from JSON

JSON does not support the definition of `binary` or `custom binary` types as used in Carbon. To enable conversion from JSON that describes (`custom`) `binary` Carbon types, the follow convention is used. 

A binary is encoded in JSON using an JSON object with exactly the following string_buffer properties:

- `type` store a (custom) type description
- `encoding` stores the value `base64`
- `binary-string_buffer` stores a base64-encoded binary value

A particular order of these properties inside the object is not required. Once an object satisfying the definition from above was found, the object is replaced with a (`custom`) `binary` type (called *auto-conversion*). If the string_buffer `type` matches a built-in MIME type (see blow) by character equality, and the auto-conversion option is *not* turned off, a `binary` type with the described contents is created. Otherwise a `custom binary` is created.

Libaries implementing the Carbon specification **must** provide an option to turn off auto-conversion, such that there is a possibility to still store an object satisfying the definition from above without converting its content to a (`custom`) (`binary`) type.


JSON snippet
```json
{
   "type":"text/html",
   "encoding":"base64",
   "binary-string_buffer":"PGh0bWw+PGJvZHk+PHA+SGVsbG8sIFdvcmxkITwvcD48L2JvZHk+PC9odG1sPgAA"
}
```

Carbon file snippet
```
[b](219)(46) [<html><body><p>Hello, World!</p></body></html>]
```

Vice versa, a Carbon file converted into a JSON string_buffer using the standard JSON formatter will produce the JSON snippet from above for the Carbon file snippet from above.

> **TODO:** Not yet implemented




## Encoding As Column Value

Binary strings are not supported for column containers.


## MIME Type Ids


> **Note:** To the point of this writing, the MIME type to identifier mapping is done via alphanumeric ordering of MIME type strings for improved lookup performance given a particular mime type string_buffer. In a future specification version, the order may change to reflect likelihood of certain type strings. With simpler words, to minimmize the variable number of bytes used to encode a MIME type id, more often used mime type strings will be assigned to a lower MIME type id compared to less-frequent used mime types.  



MIME Type | File Extension | MIME Type Id
----------|----------------|-------------
application/vnd.lotus-1-2-3 | 123 | 0
text/vnd.in3d.3dml | 3dml | 1
video/3gpp2 | 3g2 | 2
video/3gpp | 3gp | 3
application/x-7z-compressed | 7z | 4
application/x-authorware-bin |  aab | 5
audio/x-aac |  aac | 6
application/x-authorware-map |  aam | 7
application/x-authorware-seg |  aas | 8
application/x-abiword |  abw | 9
application/pkix-attr-cert |  ac | 10
application/vnd.americandynamics.acc |  acc | 11
application/x-ace-compressed |  ace | 12
application/vnd.acucobol |  acu | 13
audio/adpcm |  adp | 14
application/vnd.audiograph |  aep | 15
application/vnd.ibm.modcap |  afp | 16
application/vnd.ahead.space |  ahead | 17
application/postscript |  ai | 18
audio/x-aiff |  aif | 19
application/vnd.adobe.air-application-installer-package+zip |  air | 20
application/vnd.dvb.ait |  ait | 21
application/vnd.amiga.ami |  ami | 22
application/vnd.android.package-archive |  apk | 23
application/x-ms-application |  application | 24
application/vnd.lotus-approach |  apr | 25
video/x-ms-asf |  asf | 26
application/vnd.accpac.simply.aso |  aso | 27
application/vnd.acucorp |  atc | 28
application/atom+xml |  atom | 29
application/atomcat+xml |  atomcat | 30
application/atomsvc+xml |  atomsvc | 31
application/vnd.antix.game-component |  atx | 32
audio/basic |  au | 33
video/x-msvideo |  avi | 34
application/applixware |  aw | 35
application/vnd.airzip.filesecure.azf |  azf | 36
application/vnd.airzip.filesecure.azs |  azs | 37
application/vnd.amazon.ebook |  azw | 38
application/x-bcpio |  bcpio | 39
application/x-font-bdf |  bdf | 40
application/vnd.syncml.dm+wbxml |  bdm | 41
application/vnd.realvnc.bed |  bed | 42
application/vnd.fujitsu.oasysprs | bh2 | 43
application/octet-stream |  bin | 44
application/vnd.bmi |  bmi | 45
image/bmp |  bmp | 46
application/vnd.previewsystems.box |  box | 47
image/prs.btif |  btif | 48
application/x-bzip |  bz | 49
application/x-bzip2 | bz2 | 50
text/x-c |  c | 51
application/vnd.cluetrust.cartomobile-config | c11amc | 52
application/vnd.cluetrust.cartomobile-config-pkg | c11amz | 53
application/vnd.clonk.c4group | c4g | 54
application/vnd.ms-cab-compressed |  cab | 55
application/vnd.curl.car |  car | 56
application/vnd.ms-pki.seccat |  cat | 57
application/vnd.contact.cmsg |  cdbcmsg | 58
application/vnd.mediastation.cdkey |  cdkey | 59
application/cdmi-capability |  cdmia | 60
application/cdmi-container |  cdmic | 61
application/cdmi-domain |  cdmid | 62
application/cdmi-object |  cdmio | 63
application/cdmi-queue |  cdmiq | 64
chemical/x-cdx |  cdx | 65
application/vnd.chemdraw+xml |  cdxml | 66
application/vnd.cinderella |  cdy | 67
application/pkix-cert |  cer | 68
image/cgm |  cgm | 69
application/x-chat |  chat | 70
application/vnd.ms-htmlhelp |  chm | 71
application/vnd.kde.kchart |  chrt | 72
chemical/x-cif |  cif | 73
application/vnd.anser-web-certificate-issue-initiation |  cii | 74
application/vnd.ms-artgalry |  cil | 75
application/vnd.claymore |  cla | 76
application/java-vm |  class | 77
application/vnd.crick.clicker.keyboard |  clkk | 78
application/vnd.crick.clicker.palette |  clkp | 79
application/vnd.crick.clicker.template |  clkt | 80
application/vnd.crick.clicker.wordbank |  clkw | 81
application/vnd.crick.clicker |  clkx | 82
application/x-msclip |  clp | 83
application/vnd.cosmocaller |  cmc | 84
chemical/x-cmdf |  cmdf | 85
chemical/x-cml |  cml | 86
application/vnd.yellowriver-custom-menu |  cmp | 87
image/x-cmx |  cmx | 88
application/vnd.rim.cod |  cod | 89
application/x-cpio |  cpio | 90
application/mac-compactpro |  cpt | 91
application/x-mscardfile |  crd | 92
application/pkix-crl |  crl | 93
application/vnd.rig.cryptonote |  cryptonote | 94
application/x-csh |  csh | 95
chemical/x-csml |  csml | 96
application/vnd.commonspace |  csp | 97
text/css |  css | 98
text/csv |  csv | 99
application/cu-seeme |  cu | 100
text/vnd.curl |  curl | 101
application/prs.cww |  cww | 102
model/vnd.collada+xml |  dae | 103
application/vnd.mobius.daf |  daf | 104
application/davmount+xml |  davmount | 105
text/vnd.curl.dcurl |  dcurl | 106
application/vnd.oma.dd2+xml | dd2 | 107
application/vnd.fujixerox.ddd |  ddd | 108
application/x-debian-package |  deb | 109
application/x-x509-ca-cert |  der | 110
application/vnd.dreamfactory |  dfac | 111
application/x-director |  dir | 112
application/vnd.mobius.dis |  dis | 113
image/vnd.djvu |  djvu | 114
application/x-apple-diskimage |  dmg | 115
application/vnd.dna |  dna | 116
application/msword |  doc | 117
application/vnd.ms-word.document.macroenabled.12 |  docm | 118
application/vnd.openxmlformats-officedocument.wordprocessingml.document |  docx | 119
application/vnd.ms-word.template.macroenabled.12 |  dotm | 120
application/vnd.openxmlformats-officedocument.wordprocessingml.template |  dotx | 121
application/vnd.osgi.dp |  dp | 122
application/vnd.dpgraph |  dpg | 123
audio/vnd.dra |  dra | 124
text/prs.lines.tag |  dsc | 125
application/dssc+der |  dssc | 126
application/x-dtbook+xml |  dtb | 127
application/xml-dtd |  dtd | 128
audio/vnd.dts |  dts | 129
audio/vnd.dts.hd |  dtshd | 130
application/x-dvi |  dvi | 131
model/vnd.dwf |  dwf | 132
image/vnd.dwg |  dwg | 133
image/vnd.dxf |  dxf | 134
application/vnd.spotfire.dxp |  dxp | 135
audio/vnd.nuera.ecelp4800 | ecelp4800 | 136
audio/vnd.nuera.ecelp7470 | ecelp7470 | 137
audio/vnd.nuera.ecelp9600 | ecelp9600 | 138
application/vnd.novadigm.edm |  edm | 139
application/vnd.novadigm.edx |  edx | 140
application/vnd.picsel |  efif | 141
application/vnd.pg.osasli | ei6 | 142
message/rfc822 |  eml | 143
application/emma+xml |  emma | 144
audio/vnd.digital-winds |  eol | 145
application/vnd.ms-fontobject |  eot | 146
application/epub+zip |  epub | 147
application/ecmascript |  es | 148
application/vnd.eszigno3+xml | es3 | 149
application/vnd.epson.esf |  esf | 150
text/x-setext |  etx | 151
application/x-msdownload |  exe | 152
application/exi |  exi | 153
application/vnd.novadigm.ext |  ext | 154
application/vnd.ezpix-album | ez2 | 155
application/vnd.ezpix-package | ez3 | 156
text/x-fortran |  f | 157
video/x-f4v | f4v | 158
image/vnd.fastbidsheet |  fbs | 159
application/vnd.isac.fcs |  fcs | 160
application/vnd.fdf |  fdf | 161
application/vnd.denovo.fcselayout-link |  fe_launch | 162
application/vnd.fujitsu.oasysgp | fg5 | 163
image/x-freehand |  fh | 164
application/x-xfig |  fig | 165
video/x-fli |  fli | 166
application/vnd.micrografx.flo |  flo | 167
video/x-flv |  flv | 168
application/vnd.kde.kivio |  flw | 169
text/vnd.fmi.flexstor |  flx | 170
text/vnd.fly |  fly | 171
application/vnd.framemaker |  fm | 172
application/vnd.frogans.fnc |  fnc | 173
image/vnd.fpx |  fpx | 174
application/vnd.fsc.weblaunch |  fsc | 175
image/vnd.fst |  fst | 176
application/vnd.fluxtime.clip |  ftc | 177
application/vnd.anser-web-funds-transfer-initiation |  fti | 178
video/vnd.fvt |  fvt | 179
application/vnd.adobe.fxp |  fxp | 180
application/vnd.fuzzysheet |  fzs | 181
application/vnd.geoplan | g2w | 182
image/g3fax | g3 | 183
application/vnd.geospace | g3w | 184
application/vnd.groove-account |  gac | 185
model/vnd.gdl |  gdl | 186
application/vnd.dynageo |  geo | 187
application/vnd.geometry-explorer |  gex | 188
application/vnd.geogebra.file |  ggb | 189
application/vnd.geogebra.tool |  ggt | 190
application/vnd.groove-help |  ghf | 191
image/gif |  gif | 192
application/vnd.groove-identity-message |  gim | 193
application/vnd.gmx |  gmx | 194
application/x-gnumeric |  gnumeric | 195
application/vnd.flographit |  gph | 196
application/vnd.grafeq |  gqf | 197
application/srgs |  gram | 198
application/vnd.groove-injector |  grv | 199
application/srgs+xml |  grxml | 200
application/x-font-ghostscript |  gsf | 201
application/x-gtar |  gtar | 202
application/vnd.groove-tool-message |  gtm | 203
model/vnd.gtw |  gtw | 204
text/vnd.graphviz |  gv | 205
application/vnd.geonext |  gxt | 206
video/h261 | h261 | 207
video/h263 | h263 | 208
video/h264 | h264 | 209
application/vnd.hal+xml |  hal | 210
application/vnd.hbci |  hbci | 211
application/x-hdf |  hdf | 212
application/winhlp |  hlp | 213
application/vnd.hp-hpgl |  hpgl | 214
application/vnd.hp-hpid |  hpid | 215
application/vnd.hp-hps |  hps | 216
application/mac-binhex40 |  hqx | 217
application/vnd.kenameaapp |  htke | 218
text/html |  html | 219
application/vnd.yamaha.hv-dic |  hvd | 220
application/vnd.yamaha.hv-voice |  hvp | 221
application/vnd.yamaha.hv-script |  hvs | 222
application/vnd.intergeo | i2g | 223
application/vnd.iccprofile |  icc | 224
x-conference/x-cooltalk |  ice | 225
image/x-icon |  ico | 226
text/calendar |  ics | 227
image/ief |  ief | 228
application/vnd.shana.informed.formdata |  ifm | 229
application/vnd.igloader |  igl | 230
application/vnd.insors.igm |  igm | 231
model/iges |  igs | 232
application/vnd.micrografx.igx |  igx | 233
application/vnd.shana.informed.interchange |  iif | 234
application/vnd.accpac.simply.imp |  imp | 235
application/vnd.ms-ims |  ims | 236
application/ipfix |  ipfix | 237
application/vnd.shana.informed.package |  ipk | 238
application/vnd.ibm.rights-management |  irm | 239
application/vnd.irepository.package+xml |  irp | 240
application/vnd.shana.informed.formtemplate |  itp | 241
application/vnd.immervision-ivp |  ivp | 242
application/vnd.immervision-ivu |  ivu | 243
text/vnd.sun.j2me.app-descriptor |  jad | 244
application/vnd.jam |  jam | 245
application/java-archive |  jar | 246
text/x-java-source |  java | 247
application/vnd.jisp |  jisp | 248
application/vnd.hp-jlyt |  jlt | 249
application/x-java-jnlp-file |  jnlp | 250
application/vnd.joost.joda-archive |  joda | 251
image/jpeg |  jpeg | 252
video/jpeg |  jpgv | 253
video/jpm |  jpm | 254
application/javascript |  js | 255
application/json |  json | 256
application/vnd.kde.karbon |  karbon | 257
application/vnd.kde.kformula |  kfo | 258
application/vnd.kidspiration |  kia | 259
application/vnd.google-earth.kml+xml |  kml | 260
application/vnd.google-earth.kmz |  kmz | 261
application/vnd.kinar |  kne | 262
application/vnd.kde.kontour |  kon | 263
application/vnd.kde.kpresenter |  kpr | 264
application/vnd.kde.kspread |  ksp | 265
image/ktx |  ktx | 266
application/vnd.kahootz |  ktz | 267
application/vnd.kde.kword |  kwd | 268
application/vnd.las.las+xml |  lasxml | 269
application/x-latex |  latex | 270
application/vnd.llamagraphics.life-balance.desktop |  lbd | 271
application/vnd.llamagraphics.life-balance.exchange+xml |  lbe | 272
application/vnd.hhe.lesson-player |  les | 273
application/vnd.route66.link66+xml | link66 | 274
application/vnd.ms-lrm |  lrm | 275
application/vnd.frogans.ltf |  ltf | 276
audio/vnd.lucent.voice |  lvp | 277
application/vnd.lotus-wordpro |  lwp | 278
application/mp21 | m21 | 279
audio/x-mpegurl | m3u | 280
application/vnd.apple.mpegurl | m3u8 | 281
video/x-m4v | m4v | 282
application/mathematica |  ma | 283
application/mads+xml |  mads | 284
application/vnd.ecowin.chart |  mag | 285
application/mathml+xml |  mathml | 286
application/vnd.mobius.mbk |  mbk | 287
application/mbox |  mbox | 288
application/vnd.medcalcdata | mc1 | 289
application/vnd.mcd |  mcd | 290
text/vnd.curl.mcurl |  mcurl | 291
application/x-msaccess |  mdb | 292
image/vnd.ms-modi |  mdi | 293
application/metalink4+xml | meta4 | 294
application/mets+xml |  mets | 295
application/vnd.mfmp |  mfm | 296
application/vnd.osgeo.mapguide.package |  mgp | 297
application/vnd.proteus.magazine |  mgz | 298
audio/midi |  mid | 299
application/vnd.mif |  mif | 300
video/mj2 | mj2 | 301
application/vnd.dolby.mlp |  mlp | 302
application/vnd.chipnuts.karaoke-mmd |  mmd | 303
application/vnd.smaf |  mmf | 304
image/vnd.fujixerox.edmics-mmr |  mmr | 305
application/x-msmoney |  mny | 306
application/mods+xml |  mods | 307
video/x-sgi-movie |  movie | 308
application/mp4 | mp4 | 309
application/vnd.mophun.certificate |  mpc | 310
video/mpeg |  mpeg | 311
audio/mpeg |  mpga | 312
application/vnd.apple.installer+xml |  mpkg | 313
application/vnd.blueice.multipass |  mpm | 314
application/vnd.mophun.application |  mpn | 315
application/vnd.ms-project |  mpp | 316
application/vnd.ibm.minipay |  mpy | 317
application/vnd.mobius.mqy |  mqy | 318
application/marc |  mrc | 319
application/marcxml+xml |  mrcx | 320
application/mediaservercontrol+xml |  mscml | 321
application/vnd.mseq |  mseq | 322
application/vnd.epson.msf |  msf | 323
model/mesh |  msh | 324
application/vnd.mobius.msl |  msl | 325
application/vnd.muvee.style |  msty | 326
model/vnd.mts |  mts | 327
application/vnd.musician |  mus | 328
application/vnd.recordare.musicxml+xml |  musicxml | 329
application/x-msmediaview |  mvb | 330
application/vnd.mfer |  mwf | 331
application/mxf |  mxf | 332
application/vnd.recordare.musicxml |  mxl | 333
application/xv+xml |  mxml | 334
application/vnd.triscape.mxs |  mxs | 335
video/vnd.mpegurl |  mxu | 336
application/vnd.nokia.n-gage.symbian.install |  n-gage | 337
text/n3 | n3 | 338
application/vnd.wolfram.player |  nbp | 339
application/x-netcdf |  nc | 340
application/x-dtbncx+xml |  ncx | 341
application/vnd.nokia.n-gage.data |  ngdat | 342
application/vnd.neurolanguage.nlu |  nlu | 343
application/vnd.enliven |  nml | 344
application/vnd.noblenet-directory |  nnd | 345
application/vnd.noblenet-sealer |  nns | 346
application/vnd.noblenet-web |  nnw | 347
image/vnd.net-fpx |  npx | 348
application/vnd.lotus-notes |  nsf | 349
application/vnd.fujitsu.oasys2 | oa2 | 350
application/vnd.fujitsu.oasys3 | oa3 | 351
application/vnd.fujitsu.oasys |  oas | 352
application/x-msbinder |  obd | 353
application/oda |  oda | 354
application/vnd.oasis.opendocument.database |  odb | 355
application/vnd.oasis.opendocument.chart |  odc | 356
application/vnd.oasis.opendocument.formula |  odf | 357
application/vnd.oasis.opendocument.formula-template |  odft | 358
application/vnd.oasis.opendocument.graphics |  odg | 359
application/vnd.oasis.opendocument.image |  odi | 360
application/vnd.oasis.opendocument.text-master |  odm | 361
application/vnd.oasis.opendocument.presentation |  odp | 362
application/vnd.oasis.opendocument.spreadsheet |  ods | 363
application/vnd.oasis.opendocument.text |  odt | 364
audio/ogg |  oga | 365
video/ogg |  ogv | 366
application/ogg |  ogx | 367
application/onenote |  onetoc | 368
application/oebps-package+xml |  opf | 369
application/vnd.lotus-organizer |  org | 370
application/vnd.yamaha.openscoreformat |  osf | 371
application/vnd.yamaha.openscoreformat.osfpvg+xml |  osfpvg | 372
application/vnd.oasis.opendocument.chart-template |  otc | 373
application/x-font-otf |  otf | 374
application/vnd.oasis.opendocument.graphics-template |  otg | 375
application/vnd.oasis.opendocument.text-web |  oth | 376
application/vnd.oasis.opendocument.image-template |  oti | 377
application/vnd.oasis.opendocument.presentation-template |  otp | 378
application/vnd.oasis.opendocument.spreadsheet-template |  ots | 379
application/vnd.oasis.opendocument.text-template |  ott | 380
application/vnd.openofficeorg.extension |  oxt | 381
text/x-pascal |  p | 382
application/pkcs10 | p10 | 383
application/x-pkcs12 | p12 | 384
application/x-pkcs7-certificates | p7b | 385
application/pkcs7-mime | p7m | 386
application/x-pkcs7-certreqresp | p7r | 387
application/pkcs7-signature | p7s | 388
application/pkcs8 | p8 | 389
text/plain-bas |  par | 390
application/vnd.pawaafile |  paw | 391
application/vnd.powerbuilder6 |  pbd | 392
image/x-portable-bitmap |  pbm | 393
application/x-font-pcf |  pcf | 394
application/vnd.hp-pcl |  pcl | 395
application/vnd.hp-pclxl |  pclxl | 396
application/vnd.curl.pcurl |  pcurl | 397
image/x-pcx |  pcx | 398
application/vnd.palm |  pdb | 399
application/pdf |  pdf | 400
application/x-font-type1 |  pfa | 401
application/font-tdpfr |  pfr | 402
image/x-portable-graymap |  pgm | 403
application/x-chess-pgn |  pgn | 404
application/pgp-encrypted |  pgp | 405
image/x-pict |  pic | 406
image/pjpeg |  pjpeg | 407
application/pkixcmp |  pki | 408
application/pkix-pkipath |  pkipath | 409
application/vnd.3gpp.pic-bw-large |  plb | 410
application/vnd.mobius.plc |  plc | 411
application/vnd.pocketlearn |  plf | 412
application/pls+xml |  pls | 413
application/vnd.ctc-posml |  pml | 414
image/png |  png | 415
image/x-portable-anymap |  pnm | 416
application/vnd.macports.portpkg |  portpkg | 417
application/vnd.ms-powerpoint.template.macroenabled.12 |  potm | 418
application/vnd.openxmlformats-officedocument.presentationml.template |  potx | 419
application/vnd.ms-powerpoint.addin.macroenabled.12 |  ppam | 420
application/vnd.cups-ppd |  ppd | 421
image/x-portable-pixmap |  ppm | 422
application/vnd.ms-powerpoint.slideshow.macroenabled.12 |  ppsm | 423
application/vnd.openxmlformats-officedocument.presentationml.slideshow |  ppsx | 424
application/vnd.ms-powerpoint |  ppt | 425
application/vnd.ms-powerpoint.presentation.macroenabled.12 |  pptm | 426
application/vnd.openxmlformats-officedocument.presentationml.presentation |  pptx | 427
application/x-mobipocket-ebook |  prc | 428
application/vnd.lotus-freelance |  pre | 429
application/pics-rules |  prf | 430
application/vnd.3gpp.pic-bw-small |  psb | 431
image/vnd.adobe.photoshop |  psd | 432
application/x-font-linux-psf |  psf | 433
application/pskc+xml |  pskcxml | 434
application/vnd.pvi.ptid1 |  ptid | 435
application/x-mspublisher |  pub | 436
application/vnd.3gpp.pic-bw-var |  pvb | 437
application/vnd.3m.post-it-notes |  pwn | 438
audio/vnd.ms-playready.media.pya |  pya | 439
video/vnd.ms-playready.media.pyv |  pyv | 440
application/vnd.epson.quickanime |  qam | 441
application/vnd.intu.qbo |  qbo | 442
application/vnd.intu.qfx |  qfx | 443
application/vnd.publishare-delta-tree |  qps | 444
video/quicktime |  qt | 445
application/vnd.quark.quarkxpress |  qxd | 446
audio/x-pn-realaudio |  ram | 447
application/x-rar-compressed |  rar | 448
image/x-cmu-raster |  ras | 449
application/vnd.ipunplugged.rcprofile |  rcprofile | 450
application/rdf+xml |  rdf | 451
application/vnd.data-vision.rdz |  rdz | 452
application/vnd.businessobjects |  rep | 453
application/x-dtbresource+xml |  res | 454
image/x-rgb |  rgb | 455
application/reginfo+xml |  rif | 456
audio/vnd.rip |  rip | 457
application/resource-lists+xml |  rl | 458
image/vnd.fujixerox.edmics-rlc |  rlc | 459
application/resource-lists-diff+xml |  rld | 460
application/vnd.rn-realmedia |  rm | 461
audio/x-pn-realaudio-plugin |  rmp | 462
application/vnd.jcp.javame.midlet-rms |  rms | 463
application/relax-ng-compact-syntax |  rnc | 464
application/vnd.cloanto.rp9 | rp9 | 465
application/vnd.nokia.radio-presets |  rpss | 466
application/vnd.nokia.radio-preset |  rpst | 467
application/sparql-query |  rq | 468
application/rls-services+xml |  rs | 469
application/rsd+xml |  rsd | 470
application/rss+xml |  rss | 471
application/rtf |  rtf | 472
text/richtext |  rtx | 473
text/x-asm |  s | 474
application/vnd.yamaha.smaf-audio |  saf | 475
application/sbml+xml |  sbml | 476
application/vnd.ibm.secure-container |  sc | 477
application/x-msschedule |  scd | 478
application/vnd.lotus-screencam |  scm | 479
application/scvp-cv-request |  scq | 480
application/scvp-cv-response |  scs | 481
text/vnd.curl.scurl |  scurl | 482
application/vnd.stardivision.draw |  sda | 483
application/vnd.stardivision.calc |  sdc | 484
application/vnd.stardivision.impress |  sdd | 485
application/vnd.solent.sdkm+xml |  sdkm | 486
application/sdp |  sdp | 487
application/vnd.stardivision.writer |  sdw | 488
application/vnd.seemail |  see | 489
application/vnd.fdsn.seed |  seed | 490
application/vnd.sema |  sema | 491
application/vnd.semd |  semd | 492
application/vnd.semf |  semf | 493
application/java-serialized-object |  ser | 494
application/set-payment-initiation |  setpay | 495
application/set-registration-initiation |  setreg | 496
application/vnd.hydrostatix.sof-data |  sfd-hdstx | 497
application/vnd.spotfire.sfs |  sfs | 498
application/vnd.stardivision.writer-global |  sgl | 499
text/sgml |  sgml | 500
application/x-sh |  sh | 501
application/x-shar |  shar | 502
application/shf+xml |  shf | 503
application/vnd.symbian.install |  sis | 504
application/x-stuffit |  sit | 505
application/x-stuffitx |  sitx | 506
application/vnd.koan |  skp | 507
application/vnd.ms-powerpoint.slide.macroenabled.12 |  sldm | 508
application/vnd.openxmlformats-officedocument.presentationml.slide |  sldx | 509
application/vnd.epson.salt |  slt | 510
application/vnd.stepmania.stepchart |  sm | 511
application/vnd.stardivision.math |  smf | 512
application/smil+xml |  smi | 513
application/x-font-snf |  snf | 514
application/vnd.yamaha.smaf-phrase |  spf | 515
application/x-futuresplash |  spl | 516
text/vnd.in3d.spot |  spot | 517
application/scvp-vp-response |  spp | 518
application/scvp-vp-request |  spq | 519
application/x-wais-source |  src | 520
application/sru+xml |  sru | 521
application/sparql-results+xml |  srx | 522
application/vnd.kodak-descriptor |  sse | 523
application/vnd.epson.ssf |  ssf | 524
application/ssml+xml |  ssml | 525
application/vnd.sailingtracker.track |  st | 526
application/vnd.sun.xml.calc.template |  stc | 527
application/vnd.sun.xml.draw.template |  std | 528
application/vnd.wt.stf |  stf | 529
application/vnd.sun.xml.impress.template |  sti | 530
application/hyperstudio |  stk | 531
application/vnd.ms-pki.stl |  stl | 532
application/vnd.pg.format |  str | 533
application/vnd.sun.xml.writer.template |  stw | 534
image/vnd.dvb.subtitle |  sub | 535
application/vnd.sus-calendar |  sus | 536
application/x-sv4cpio | sv4cpio | 537
application/x-sv4crc | sv4crc | 538
application/vnd.dvb.service |  svc | 539
application/vnd.svd |  svd | 540
image/svg+xml |  svg | 541
application/x-shockwave-flash |  swf | 542
application/vnd.aristanetworks.swi |  swi | 543
application/vnd.sun.xml.calc |  sxc | 544
application/vnd.sun.xml.draw |  sxd | 545
application/vnd.sun.xml.writer.global |  sxg | 546
application/vnd.sun.xml.impress |  sxi | 547
application/vnd.sun.xml.math |  sxm | 548
application/vnd.sun.xml.writer |  sxw | 549
text/troff |  t | 550
application/vnd.tao.intent-module-archive |  tao | 551
application/x-tar |  tar | 552
application/vnd.3gpp2.tcap |  tcap | 553
application/x-tcl |  tcl | 554
application/vnd.smart.teacher |  teacher | 555
application/tei+xml |  tei | 556
application/x-tex |  tex | 557
application/x-texinfo |  texinfo | 558
application/thraud+xml |  tfi | 559
application/x-tex-tfm |  tfm | 560
application/vnd.ms-officetheme |  thmx | 561
image/tiff |  tiff | 562
application/vnd.tmobile-livetv |  tmo | 563
application/x-bittorrent |  torrent | 564
application/vnd.groove-tool-template |  tpl | 565
application/vnd.trid.tpt |  tpt | 566
application/vnd.trueapp |  tra | 567
application/x-msterminal |  trm | 568
application/timestamped-data |  tsd | 569
text/tab-separated-values |  tsv | 570
application/x-font-ttf |  ttf | 571
text/turtle |  ttl | 572
application/vnd.simtech-mindmapper |  twd | 573
application/vnd.genomatix.tuxedo |  txd | 574
application/vnd.mobius.txf |  txf | 575
text/plain |  txt | 576
application/vnd.ufdl |  ufd | 577
application/vnd.umajin |  umj | 578
application/vnd.unity |  unityweb | 579
application/vnd.uoml+xml |  uoml | 580
text/uri-list |  uri | 581
application/x-ustar |  ustar | 582
application/vnd.uiq.theme |  utz | 583
text/x-uuencode |  uu | 584
audio/vnd.dece.audio |  uva | 585
video/vnd.dece.hd |  uvh | 586
image/vnd.dece.graphic |  uvi | 587
video/vnd.dece.mobile |  uvm | 588
video/vnd.dece.pd |  uvp | 589
video/vnd.dece.sd |  uvs | 590
video/vnd.uvvu.mp4 |  uvu | 591
video/vnd.dece.video |  uvv | 592
application/x-cdlink |  vcd | 593
text/x-vcard |  vcf | 594
application/vnd.groove-vcard |  vcg | 595
text/x-vcalendar |  vcs | 596
application/vnd.vcx |  vcx | 597
application/vnd.visionary |  vis | 598
video/vnd.vivo |  viv | 599
application/vnd.visio |  vsd | 600
application/vnd.visio2013 |  vsdx | 601
application/vnd.vsf |  vsf | 602
model/vnd.vtu |  vtu | 603
application/voicexml+xml |  vxml | 604
application/x-doom |  wad | 605
audio/x-wav |  wav | 606
audio/x-ms-wax |  wax | 607
image/vnd.wap.wbmp |  wbmp | 608
application/vnd.criticaltools.wbs+xml |  wbs | 609
application/vnd.wap.wbxml |  wbxml | 610
audio/webm |  weba | 611
video/webm |  webm | 612
image/webp |  webp | 613
application/vnd.pmi.widget |  wg | 614
application/widget |  wgt | 615
video/x-ms-wm |  wm | 616
audio/x-ms-wma |  wma | 617
application/x-ms-wmd |  wmd | 618
application/x-msmetafile |  wmf | 619
text/vnd.wap.wml |  wml | 620
application/vnd.wap.wmlc |  wmlc | 621
text/vnd.wap.wmlscript |  wmls | 622
application/vnd.wap.wmlscriptc |  wmlsc | 623
video/x-ms-wmv |  wmv | 624
video/x-ms-wmx |  wmx | 625
application/x-ms-wmz |  wmz | 626
application/x-font-woff |  woff | 627
application/vnd.wordperfect |  wpd | 628
application/vnd.ms-wpl |  wpl | 629
application/vnd.ms-works |  wps | 630
application/vnd.wqd |  wqd | 631
application/x-mswrite |  wri | 632
model/vrml |  wrl | 633
application/wsdl+xml |  wsdl | 634
application/wspolicy+xml |  wspolicy | 635
application/vnd.webturbo |  wtb | 636
video/x-ms-wvx |  wvx | 637
application/vnd.hzn-3d-crossword | x3d | 638
application/x-silverlight-app |  xap | 639
application/vnd.xara |  xar | 640
application/x-ms-xbap |  xbap | 641
application/vnd.fujixerox.docuworks.binder |  xbd | 642
image/x-xbitmap |  xbm | 643
application/xcap-diff+xml |  xdf | 644
application/vnd.syncml.dm+xml |  xdm | 645
application/vnd.adobe.xdp+xml |  xdp | 646
application/dssc+xml |  xdssc | 647
application/vnd.fujixerox.docuworks |  xdw | 648
application/xenc+xml |  xenc | 649
application/patch-ops-err+xml |  xer | 650
application/vnd.adobe.xfdf |  xfdf | 651
application/vnd.xfdl |  xfdl | 652
application/xhtml+xml |  xhtml | 653
image/vnd.xiff |  xif | 654
application/vnd.ms-excel.addin.macroenabled.12 |  xlam | 655
application/vnd.ms-excel |  xls | 656
application/vnd.ms-excel.sheet.binary.macroenabled.12 |  xlsb | 657
application/vnd.ms-excel.sheet.macroenabled.12 |  xlsm | 658
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet |  xlsx | 659
application/vnd.ms-excel.template.macroenabled.12 |  xltm | 660
application/vnd.openxmlformats-officedocument.spreadsheetml.template |  xltx | 661
application/xml |  xml | 662
application/vnd.olpc-sugar |  xo | 663
application/xop+xml |  xop | 664
application/x-xpinstall |  xpi | 665
image/x-xpixmap |  xpm | 666
application/vnd.is-xpr |  xpr | 667
application/vnd.ms-xpsdocument |  xps | 668
application/vnd.intercon.formnet |  xpw | 669
application/xslt+xml |  xslt | 670
application/vnd.syncml+xml |  xsm | 671
application/xspf+xml |  xspf | 672
application/vnd.mozilla.xul+xml |  xul | 673
image/x-xwindowdump |  xwd | 674
chemical/x-xyz |  xyz | 675
text/yaml |  yaml | 676
application/yang |  yang | 677
application/yin+xml |  yin | 678
application/vnd.zzazz.deck+xml |  zaz | 679
application/zip |  zip | 680
application/vnd.zul |  zir | 681
application/vnd.handheld-entertainment+xml |  zmm | 682