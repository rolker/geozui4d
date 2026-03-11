# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#:0 [SESSION BEGUN]
set subWindowCount 0
global stereoflag
set stereoflag -1
global toglPath
global dominantEye
set dominantEye 0
global oneEyedFlag
set oneEyedFlag 0
global viewMovesWithWindow
set viewMovesWithWindow 0
global GeoZuiVersion
set GeoZuiVersion 3.3
global zoomInterface
set zoomInterface 2
global recorderResolution
set recorderResolution 25
global widgetCompass
set widgetCompass 1
global widgetHScale
set widgetHScale 1
global widgetVScale
set widgetVScale 1
global widgetLighting
set widgetLighting 0
global widgetShallow
set widgetShallow 0
global drewLowRes
set drewLowRes 0
wm geometry . 400x300+110+110
#:0 [Sync]
.glwindow motion 135 8 1 0
.glwindow motion 127 2 1 0
#:0.032 [Sync]
#:0.063 [Sync]
#:0.094 [Sync]
.glwindow motion 101 -17 1 -1
.glwindow keyboard 94 -24 1 * 1 42
#:0.125 [Sync]
.glwindow keyboard 88 -26 1 {} 1 65505
#:0.157 [Sync]
#:0.188 [Sync]
#:0.219 [Sync]
#:0.25 [Sync]
#:0.282 [Sync]
#:0.313 [Sync]
#:0.344 [Sync]
#:0.375 [Sync]
#:0.407 [Sync]
#:0.438 [Sync]
#:0.469 [Sync]
#:0.5 [Sync]
#:0.532 [Sync]
#:0.594 [Sync]
#:0.625 [Sync]
#:0.657 [Sync]
#:0.688 [Sync]
#:0.719 [Sync]
#:0.75 [Sync]
#:0.782 [Sync]
#:0.813 [Sync]
#:0.844 [Sync]
#:0.875 [Sync]
#:0.907 [Sync]
#:0.938 [Sync]
#:0.969 [Sync]
#:1 [Sync]
#:1.032 [Sync]
#:1.063 [Sync]
#:1.094 [Sync]
#:1.125 [Sync]
#:1.157 [Sync]
#:1.188 [Sync]
#:1.219 [Sync]
#:1.25 [Sync]
#:1.282 [Sync]
#:1.313 [Sync]
#:1.344 [Sync]
#:1.375 [Sync]
#:1.407 [Sync]
.glwindow getObjectExtensions
.glwindow getObjectTypes
#:1.532 [Sync]
wm geometry . 400x300+110+110
#:2 [Sync]
#:2.032 [Sync]
#:2.063 [Sync]
#:2.094 [Sync]
#:2.125 [Sync]
#:2.157 [Sync]
#:2.188 [Sync]
#:2.219 [Sync]
#:2.25 [Sync]
#:2.282 [Sync]
#:2.313 [Sync]
#:2.344 [Sync]
#:2.375 [Sync]
#:2.407 [Sync]
#:2.438 [Sync]
#:2.469 [Sync]
#:2.5 [Sync]
#:2.532 [Sync]
#:2.579 [Sync]
#:2.61 [Sync]
#:2.641 [Sync]
#:2.672 [Sync]
#:2.704 [Sync]
#:2.735 [Sync]
#:2.766 [Sync]
#:2.797 [Sync]
#:2.829 [Sync]
#:2.86 [Sync]
#:2.891 [Sync]
#:2.922 [Sync]
#:2.954 [Sync]
#:2.985 [Sync]
#:3.016 [Sync]
#:3.047 [Sync]
#:3.079 [Sync]
#:3.11 [Sync]
#:3.141 [Sync]
#:3.172 [Sync]
#:3.204 [Sync]
wm geometry . 400x300+110+110
.glwindow loadobject C:/UserFiles/mdp/demo/data/nonan.sdv
.glwindow posable set Object.nonan_gutm Hxyz 0.000000 0.000000 0.000000
.glwindow posable set Object.r509_gutm xyz 9493.000000 15030.000000 200.000000
.glwindow posable set Object.r509_gutm Hxyz 9493.000000 15030.000000 200.000000
.glwindow posable set Object.ROV4_tube xyz 9615.000000 15220.000000 600.000000
.glwindow posable set Object.ROV4_tube Hxyz 9615.000000 15220.000000 600.000000
.glwindow posable set Object.cropped509_img xyz 9634.000000 15216.000000 530.000000
.glwindow posable set Object.cropped509_img Hxyz 9634.000000 15216.000000 530.000000
#:4.407 [Sync]
.glwindow motion 226 147 0 1
.glwindow motion 231 147 0 0
#:4.438 [Sync]
.glwindow motion 233 147 0 0
#:4.469 [Sync]
.glwindow motion 238 147 0 0
.glwindow motion 240 145 0 0
#:4.5 [Sync]
.glwindow motion 245 143 0 0
.glwindow motion 251 138 0 0
#:4.532 [Sync]
.glwindow motion 256 136 0 0
.glwindow motion 257 136 0 0
#:4.563 [Sync]
.glwindow motion 258 136 0 0
.glwindow motion 259 135 0 0
#:4.594 [Sync]
#:4.625 [Sync]
.glwindow motion 260 134 0 0
.glwindow motion 261 133 0 0
#:4.657 [Sync]
.glwindow motion 263 132 0 0
.glwindow motion 264 131 0 0
#:4.688 [Sync]
.glwindow motion 264 130 0 0
.glwindow motion 265 130 0 0
#:4.719 [Sync]
#:4.75 [Sync]
#:4.782 [Sync]
#:4.813 [Sync]
#:4.844 [Sync]
#:4.875 [Sync]
#:4.907 [Sync]
#:4.938 [Sync]
.glwindow motion 266 130 0 0
.glwindow motion 272 129 0 0
#:4.969 [Sync]
.glwindow motion 280 121 0 0
#:5 [Sync]
.glwindow motion 289 110 0 0
.glwindow motion 299 99 0 0
#:5.032 [Sync]
.glwindow motion 310 88 0 0
.glwindow motion 319 76 0 0
#:5.063 [Sync]
.glwindow motion 328 66 0 0
.glwindow motion 336 58 0 0
#:5.094 [Sync]
.glwindow motion 338 52 0 0
.glwindow motion 339 46 0 0
#:5.125 [Sync]
.glwindow motion 339 41 0 0
#:5.157 [Sync]
.glwindow motion 339 39 0 0
.glwindow motion 339 38 0 0
#:5.188 [Sync]
.glwindow motion 339 36 0 0
.glwindow motion 339 31 0 0
#:5.219 [Sync]
.glwindow motion 339 29 0 0
.glwindow motion 340 27 0 0
#:5.25 [Sync]
.glwindow motion 342 21 0 0
#:5.282 [Sync]
.glwindow motion 347 13 0 0
.glwindow motion 355 6 0 0
#:5.313 [Sync]
.glwindow motion 362 4 0 0
.glwindow motion 367 2 0 0
#:5.344 [Sync]
#:5.375 [Sync]
#:5.407 [Sync]
.glwindow motion 370 -8 0 -1
#:5.438 [Sync]
#:5.469 [Sync]
#:5.5 [Sync]
#:5.532 [Sync]
#:5.594 [Sync]
#:5.625 [Sync]
#:5.657 [Sync]
#:5.688 [Sync]
#:5.719 [Sync]
#:5.75 [Sync]
#:5.782 [Sync]
#:5.813 [Sync]
#:5.844 [Sync]
#:5.875 [Sync]
#:5.907 [Sync]
wm geometry . 1024x730+-4+-4
#:6.016 [Sync]
.glwindow motion 486 86 0 1
#:6.047 [Sync]
#:6.079 [Sync]
#:6.11 [Sync]
.glwindow motion 485 86 0 0
.glwindow motion 483 88 0 0
#:6.141 [Sync]
.glwindow motion 478 93 0 0
#:6.172 [Sync]
.glwindow motion 472 100 0 0
.glwindow motion 465 106 0 0
#:6.204 [Sync]
.glwindow motion 458 111 0 0
.glwindow motion 456 113 0 0
#:6.235 [Sync]
.glwindow motion 454 114 0 0
.glwindow motion 453 115 0 0
#:6.266 [Sync]
.glwindow motion 452 116 0 0
.glwindow motion 452 117 0 0
#:6.297 [Sync]
.glwindow motion 453 119 0 0
#:6.329 [Sync]
.glwindow motion 460 121 0 0
.glwindow motion 486 122 0 0
#:6.36 [Sync]
.glwindow motion 516 123 0 0
.glwindow motion 546 123 0 0
#:6.391 [Sync]
.glwindow motion 555 123 0 0
.glwindow motion 560 124 0 0
#:6.422 [Sync]
.glwindow motion 561 124 0 0
#:6.454 [Sync]
#:6.485 [Sync]
.glwindow motion 562 125 0 0
.glwindow motion 563 126 0 0
#:6.516 [Sync]
.glwindow motion 564 127 0 0
.glwindow motion 564 129 0 0
#:6.547 [Sync]
.glwindow motion 564 131 0 0
.glwindow motion 564 133 0 0
#:6.579 [Sync]
.glwindow motion 563 135 0 0
.glwindow motion 558 137 0 0
#:6.61 [Sync]
.glwindow motion 556 138 0 0
#:6.641 [Sync]
.glwindow motion 555 139 0 0
.glwindow motion 553 141 0 0
#:6.672 [Sync]
.glwindow motion 552 142 0 0
.glwindow motion 551 143 0 0
#:6.704 [Sync]
.glwindow motion 550 143 0 0
#:6.735 [Sync]
.glwindow motion 549 143 0 0
#:6.766 [Sync]
.glwindow motion 549 144 0 0
#:6.797 [Sync]
.glwindow motion 548 145 0 0
.glwindow motion 548 146 0 0
#:6.829 [Sync]
.glwindow motion 547 147 0 0
#:6.86 [Sync]
.glwindow motion 547 148 0 0
#:6.891 [Sync]
#:6.922 [Sync]
.glwindow motion 546 148 0 0
.glwindow motion 546 149 0 0
#:6.954 [Sync]
#:6.985 [Sync]
#:7.016 [Sync]
.glwindow mouse 546 149 0 1 0
#:7.047 [Sync]
.glwindow motion 546 150 256 0
#:7.079 [Sync]
.glwindow posable set Window.main p 0.500000
#:7.125 [Sync]
.glwindow motion 546 152 256 0
.glwindow posable set Window.main p 1.500000
#:7.157 [Sync]
.glwindow motion 547 167 256 0
.glwindow posable set Window.main p 9.000000
#:7.188 [Sync]
.glwindow motion 549 179 256 0
.glwindow posable set Window.main p 15.000000
#:7.219 [Sync]
.glwindow motion 549 184 256 0
.glwindow posable set Window.main p 17.500000
#:7.25 [Sync]
.glwindow motion 550 192 256 0
.glwindow posable set Window.main p 21.500000
#:7.282 [Sync]
.glwindow motion 550 194 256 0
.glwindow posable set Window.main p 22.500000
#:7.313 [Sync]
.glwindow motion 550 195 256 0
.glwindow posable set Window.main p 23.000000
#:7.344 [Sync]
.glwindow motion 550 196 256 0
.glwindow posable set Window.main p 23.500000
#:7.375 [Sync]
#:7.407 [Sync]
.glwindow motion 549 198 256 0
.glwindow posable set Window.main p 24.500000
#:7.438 [Sync]
.glwindow motion 549 199 256 0
.glwindow posable set Window.main p 25.000000
.glwindow mouse 549 199 256 1 1
#:7.469 [Sync]
.glwindow motion 550 200 0 0
.glwindow motion 552 200 0 0
#:7.5 [Sync]
.glwindow motion 559 200 0 0
#:7.532 [Sync]
.glwindow motion 604 200 0 0
.glwindow motion 638 199 0 0
#:7.579 [Sync]
.glwindow motion 670 197 0 0
.glwindow motion 702 196 0 0
#:7.61 [Sync]
.glwindow motion 730 194 0 0
#:7.641 [Sync]
.glwindow motion 756 192 0 0
.glwindow motion 782 190 0 0
#:7.672 [Sync]
.glwindow motion 793 188 0 0
.glwindow motion 819 186 0 0
#:7.704 [Sync]
.glwindow motion 845 184 0 0
.glwindow motion 873 179 0 0
#:7.735 [Sync]
.glwindow motion 905 173 0 0
.glwindow motion 941 164 0 0
#:7.766 [Sync]
.glwindow motion 952 156 0 0
#:7.797 [Sync]
.glwindow motion 953 155 0 0
.glwindow motion 954 154 0 0
#:7.829 [Sync]
.glwindow motion 954 149 0 0
.glwindow motion 954 142 0 0
#:7.86 [Sync]
.glwindow motion 955 136 0 0
.glwindow motion 955 131 0 0
#:7.891 [Sync]
.glwindow motion 955 130 0 0
#:7.922 [Sync]
#:7.954 [Sync]
.glwindow motion 955 132 0 0
.glwindow motion 954 142 512 0
#:7.985 [Sync]
.glwindow posable set Window.main s 0.920000
.glwindow posable set Window.main xyz 308.028015 261.104858 57.184917
.glwindow motion 953 154 512 0
.glwindow posable set Window.main s 0.846400
.glwindow posable set Window.main xyz 539.049011 456.933502 100.073608
#:8.016 [Sync]
.glwindow motion 952 180 512 0
.glwindow posable set Window.main s 0.778688
.glwindow posable set Window.main xyz 712.314758 603.804993 132.240128
#:8.047 [Sync]
.glwindow motion 949 236 512 0
.glwindow posable set Window.main s 0.716393
.glwindow posable set Window.main xyz 842.264099 713.958618 156.365021
#:8.079 [Sync]
.glwindow motion 947 276 512 0
.glwindow posable set Window.main s 0.659082
.glwindow posable set Window.main xyz 939.726074 796.573853 174.458679
#:8.11 [Sync]
.glwindow motion 946 288 512 0
.glwindow posable set Window.main s 0.606355
.glwindow posable set Window.main xyz 1012.822571 858.535278 188.028931
#:8.141 [Sync]
.glwindow motion 945 302 512 0
.glwindow posable set Window.main s 0.557847
.glwindow posable set Window.main xyz 1067.644897 905.006348 198.206619
#:8.172 [Sync]
.glwindow motion 944 303 512 0
.glwindow posable set Window.main s 0.513219
.glwindow posable set Window.main xyz 1108.761719 939.859619 205.839890
#:8.204 [Sync]
.glwindow motion 944 304 512 0
.glwindow posable set Window.main s 0.472161
.glwindow posable set Window.main xyz 1139.599365 965.999573 211.564835
#:8.235 [Sync]
.glwindow motion 943 304 512 0
.glwindow posable set Window.main s 0.434389
.glwindow posable set Window.main xyz 1162.727539 985.604553 215.858551
#:8.266 [Sync]
.glwindow posable set Window.main s 0.399637
.glwindow posable set Window.main xyz 1180.073730 1000.308289 219.078827
#:8.297 [Sync]
.glwindow motion 943 306 512 0
.glwindow posable set Window.main s 0.367666
.glwindow posable set Window.main xyz 1193.083374 1011.336060 221.494034
#:8.329 [Sync]
.glwindow motion 942 307 512 0
.glwindow posable set Window.main s 0.338253
.glwindow posable set Window.main xyz 1202.840576 1019.606934 223.305450
#:8.36 [Sync]
.glwindow motion 942 308 512 0
.glwindow posable set Window.main s 0.311193
.glwindow posable set Window.main xyz 1210.158447 1025.810059 224.664001
#:8.391 [Sync]
.glwindow posable set Window.main s 0.286298
.glwindow posable set Window.main xyz 1215.646851 1030.462402 225.682922
#:8.422 [Sync]
.glwindow posable set Window.main s 0.263394
.glwindow posable set Window.main xyz 1219.763184 1033.951660 226.447113
#:8.454 [Sync]
.glwindow posable set Window.main s 0.242322
.glwindow posable set Window.main xyz 1222.850464 1036.568604 227.020248
#:8.485 [Sync]
.glwindow motion 941 308 512 0
.glwindow posable set Window.main s 0.222936
.glwindow posable set Window.main xyz 1225.165894 1038.531372 227.450104
#:8.516 [Sync]
.glwindow posable set Window.main s 0.205102
.glwindow posable set Window.main xyz 1226.902466 1040.003418 227.772491
#:8.547 [Sync]
.glwindow posable set Window.main s 0.188693
.glwindow posable set Window.main xyz 1228.204956 1041.107422 228.014282
#:8.579 [Sync]
.glwindow posable set Window.main s 0.173598
.glwindow posable set Window.main xyz 1229.181763 1041.935425 228.195633
#:8.61 [Sync]
.glwindow posable set Window.main s 0.159710
.glwindow posable set Window.main xyz 1229.914429 1042.556396 228.331635
#:8.641 [Sync]
.glwindow posable set Window.main s 0.146933
.glwindow posable set Window.main xyz 1230.463867 1043.022217 228.433640
#:8.672 [Sync]
.glwindow posable set Window.main s 0.135179
.glwindow posable set Window.main xyz 1230.875977 1043.371582 228.510147
#:8.704 [Sync]
.glwindow motion 940 307 512 0
.glwindow motion 938 306 0 0
#:8.735 [Sync]
.glwindow motion 938 305 0 0
.glwindow motion 936 304 0 0
#:8.766 [Sync]
.glwindow motion 929 302 0 0
.glwindow motion 920 296 0 0
#:8.797 [Sync]
.glwindow motion 910 290 0 0
.glwindow motion 898 284 0 0
#:8.829 [Sync]
.glwindow motion 887 279 0 0
#:8.86 [Sync]
.glwindow motion 879 277 0 0
.glwindow motion 877 272 0 0
#:8.891 [Sync]
.glwindow motion 876 267 0 0
.glwindow motion 876 262 0 0
#:8.922 [Sync]
.glwindow motion 876 261 0 0
#:8.954 [Sync]
#:8.985 [Sync]
#:9.016 [Sync]
#:9.047 [Sync]
#:9.079 [Sync]
.glwindow posable set Window.main xyz 2906.915039 2151.562012 241.937012
#:9.11 [Sync]
.glwindow posable set Window.main xyz 4163.944336 2982.704834 252.007156
#:9.141 [Sync]
.glwindow posable set Window.main xyz 5106.716309 3606.062012 259.559784
#:9.172 [Sync]
#:9.204 [Sync]
#:9.235 [Sync]
#:9.266 [Sync]
.glwindow motion 875 261 0 0
.glwindow motion 870 262 0 0
#:9.297 [Sync]
.glwindow motion 862 263 0 0
.glwindow motion 851 269 0 0
#:9.329 [Sync]
.glwindow motion 825 277 0 0
.glwindow motion 813 287 0 0
#:9.36 [Sync]
.glwindow motion 801 296 0 0
.glwindow motion 789 303 0 0
#:9.391 [Sync]
.glwindow motion 780 308 0 0
#:9.422 [Sync]
.glwindow motion 774 309 0 0
.glwindow motion 773 310 0 0
#:9.454 [Sync]
#:9.485 [Sync]
#:9.516 [Sync]
#:9.547 [Sync]
#:9.579 [Sync]
#:9.61 [Sync]
#:9.641 [Sync]
#:9.672 [Sync]
#:9.704 [Sync]
#:9.735 [Sync]
#:9.766 [Sync]
#:9.797 [Sync]
#:9.829 [Sync]
#:9.86 [Sync]
.glwindow keyboard 773 310 0 z 0 122
.glwindow motion 773 303 0 0
#:9.891 [Sync]
.glwindow motion 773 293 0 0
.glwindow motion 774 267 0 0
.glwindow keyboard 774 267 0 z 1 122
#:9.922 [Sync]
.glwindow motion 775 235 0 0
.glwindow motion 775 205 0 0
#:9.954 [Sync]
.glwindow motion 775 195 0 0
#:9.985 [Sync]
.glwindow motion 775 190 0 0
.glwindow motion 775 188 0 0
#:10.016 [Sync]
.glwindow motion 774 187 0 0
.glwindow motion 773 185 0 0
#:10.047 [Sync]
.glwindow motion 772 184 0 0
#:10.079 [Sync]
.glwindow motion 772 183 0 0
.glwindow motion 771 182 0 0
#:10.11 [Sync]
.glwindow motion 769 180 0 0
#:10.141 [Sync]
.glwindow motion 767 175 0 0
.glwindow motion 759 174 0 0
#:10.172 [Sync]
.glwindow motion 748 172 0 0
.glwindow motion 718 167 0 0
#:10.204 [Sync]
.glwindow motion 706 166 0 0
.glwindow motion 700 165 0 0
#:10.235 [Sync]
#:10.266 [Sync]
#:10.297 [Sync]
#:10.329 [Sync]
.glwindow motion 701 164 0 0
#:10.36 [Sync]
.glwindow motion 701 163 0 0
#:10.391 [Sync]
.glwindow motion 701 162 0 0
#:10.422 [Sync]
.glwindow motion 701 161 0 0
.glwindow motion 701 160 0 0
#:10.454 [Sync]
.glwindow motion 700 159 0 0
.glwindow motion 700 158 0 0
#:10.485 [Sync]
#:10.516 [Sync]
#:10.547 [Sync]
#:10.579 [Sync]
.glwindow motion 700 157 0 0
.glwindow motion 699 157 0 0
#:10.61 [Sync]
.glwindow motion 698 156 0 0
#:10.641 [Sync]
.glwindow motion 697 155 0 0
#:10.672 [Sync]
#:10.704 [Sync]
.glwindow motion 696 155 0 0
#:10.735 [Sync]
#:10.766 [Sync]
.glwindow motion 696 154 0 0
#:10.797 [Sync]
#:10.829 [Sync]
#:10.86 [Sync]
#:10.891 [Sync]
#:10.922 [Sync]
#:10.954 [Sync]
#:10.985 [Sync]
#:11.016 [Sync]
#:11.047 [Sync]
#:11.079 [Sync]
#:11.11 [Sync]
#:11.141 [Sync]
#:11.172 [Sync]
#:11.204 [Sync]
#:11.235 [Sync]
.glwindow mouse 696 154 0 1 0
#:11.266 [Sync]
#:11.297 [Sync]
.glwindow mouse 696 154 256 1 1
.glwindow posable set ve Hsxyzphr 1.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
.glwindow posable set Window.sub0.Light xyz 1.000000 1.000000 1.000000
.glwindow posable set Window.sub0.Light Hxyz 1.000000 1.000000 1.000000
.sub0.innards.togl positionUpdated 0 0 380 285
.glwindow posable set Window.sub0 xyz 9615.714844 15229.518555 542.649597
.glwindow posable set Window.sub0 s 0.375083
.glwindow posable set Window.sub0 Hsxyzphr 0.375083 9615.714844 15229.518555 542.649597 25.000000 0.000000 0.000000
#:11.485 [Sync]
#:11.547 [Sync]
#:11.579 [Sync]
#:11.61 [Sync]
#:11.641 [Sync]
#:11.672 [Sync]
#:11.704 [Sync]
#:11.735 [Sync]
#:11.766 [Sync]
.glwindow motion 690 154 0 0
#:11.797 [Sync]
.glwindow motion 656 153 0 0
#:11.829 [Sync]
.glwindow motion 600 152 0 0
.glwindow motion 538 152 0 0
#:11.86 [Sync]
.glwindow motion 484 157 0 0
.glwindow motion 432 164 0 0
#:11.891 [Sync]
.glwindow motion 386 170 0 0
.glwindow motion 354 175 0 -1
.sub0.innards.togl motion 351 153 0 1
#:11.922 [Sync]
.sub0.innards.togl motion 344 155 0 0
.sub0.innards.togl motion 344 156 0 0
#:11.954 [Sync]
.sub0.innards.togl motion 345 157 0 0
#:11.985 [Sync]
.sub0.innards.togl motion 347 163 0 0
.sub0.innards.togl motion 349 172 0 0
#:12.016 [Sync]
.sub0.innards.togl motion 350 181 0 0
.sub0.innards.togl motion 351 189 0 0
#:12.047 [Sync]
.sub0.innards.togl motion 350 191 0 0
.sub0.innards.togl motion 345 192 0 0
#:12.079 [Sync]
.sub0.innards.togl motion 339 193 0 0
#:12.11 [Sync]
.sub0.innards.togl motion 333 193 0 0
.sub0.innards.togl motion 322 193 0 0
#:12.141 [Sync]
.sub0.innards.togl motion 290 193 0 0
.sub0.innards.togl motion 254 188 0 0
#:12.172 [Sync]
.sub0.innards.togl motion 218 179 0 0
.sub0.innards.togl motion 174 147 0 0
#:12.204 [Sync]
.sub0.innards.togl motion 132 111 0 0
.sub0.innards.togl motion 96 71 0 0
#:12.235 [Sync]
.sub0.innards.togl motion 84 41 0 0
#:12.266 [Sync]
.sub0.innards.togl motion 75 31 0 0
.sub0.innards.togl motion 70 22 0 0
#:12.297 [Sync]
.sub0.innards.togl motion 69 16 0 0
.sub0.innards.togl motion 69 11 0 0
#:12.329 [Sync]
.sub0.innards.togl motion 69 10 0 0
#:12.36 [Sync]
.sub0.innards.togl motion 69 9 0 0
#:12.391 [Sync]
.sub0.innards.togl motion 70 9 0 0
#:12.422 [Sync]
.sub0.innards.togl motion 71 9 0 0
#:12.454 [Sync]
#:12.485 [Sync]
.sub0.innards.togl motion 72 8 0 0
#:12.516 [Sync]
.sub0.innards.togl motion 72 7 0 0
.sub0.innards.togl motion 73 2 0 0
#:12.547 [Sync]
.sub0.innards.togl motion 74 0 0 0
#:12.579 [Sync]
.sub0.innards.togl motion 75 -2 0 -1
#:12.61 [Sync]
#:12.641 [Sync]
#:12.672 [Sync]
#:12.704 [Sync]
#:12.735 [Sync]
#:12.766 [Sync]
#:12.797 [Sync]
#:12.829 [Sync]
 subwindowMoveMouseStart sub0 79 56
#:12.86 [Sync]
#:12.891 [Sync]
#:12.922 [Sync]
 subwindowMoveMouseMove sub0 80 56
.sub0.innards.togl positionUpdated 1 0
 subwindowMoveMouseMove sub0 82 58
.sub0.innards.togl positionUpdated 3 2
#:12.985 [Sync]
 subwindowMoveMouseMove sub0 85 62
.sub0.innards.togl positionUpdated 6 6
#:13.016 [Sync]
 subwindowMoveMouseMove sub0 88 64
.sub0.innards.togl positionUpdated 9 8
 subwindowMoveMouseMove sub0 89 65
.sub0.innards.togl positionUpdated 10 9
#:13.063 [Sync]
 subwindowMoveMouseMove sub0 90 66
.sub0.innards.togl positionUpdated 11 10
 subwindowMoveMouseMove sub0 94 73
.sub0.innards.togl positionUpdated 15 17
#:13.11 [Sync]
 subwindowMoveMouseMove sub0 95 75
.sub0.innards.togl positionUpdated 16 19
#:13.141 [Sync]
 subwindowMoveMouseMove sub0 97 78
.sub0.innards.togl positionUpdated 18 22
 subwindowMoveMouseMove sub0 98 79
.sub0.innards.togl positionUpdated 19 23
#:13.172 [Sync]
 subwindowMoveMouseMove sub0 99 81
.sub0.innards.togl positionUpdated 20 25
#:13.204 [Sync]
 subwindowMoveMouseMove sub0 99 82
.sub0.innards.togl positionUpdated 20 26
#:13.235 [Sync]
 subwindowMoveMouseMove sub0 99 83
.sub0.innards.togl positionUpdated 20 27
 subwindowMoveMouseMove sub0 99 84
.sub0.innards.togl positionUpdated 20 28
#:13.266 [Sync]
 subwindowMoveMouseMove sub0 98 85
.sub0.innards.togl positionUpdated 19 29
#:13.297 [Sync]
 subwindowMoveMouseMove sub0 97 86
.sub0.innards.togl positionUpdated 18 30
 subwindowMoveMouseStop sub0 97 86
.sub0.innards.togl positionUpdated 18 30
#:13.329 [Sync]
#:13.36 [Sync]
#:13.391 [Sync]
#:13.422 [Sync]
.sub0.innards.togl motion 79 8 0 1
#:13.454 [Sync]
.sub0.innards.togl motion 111 40 0 0
#:13.485 [Sync]
.sub0.innards.togl motion 157 80 0 0
.sub0.innards.togl motion 209 128 0 0
#:13.516 [Sync]
.sub0.innards.togl motion 265 180 0 0
.sub0.innards.togl motion 319 228 0 0
#:13.547 [Sync]
.sub0.innards.togl motion 363 264 0 -1
.glwindow motion 384 316 0 1
.glwindow motion 414 325 0 0
#:13.579 [Sync]
.glwindow motion 421 326 0 0
#:13.61 [Sync]
#:13.641 [Sync]
#:13.672 [Sync]
.glwindow motion 422 326 0 0
.glwindow motion 422 325 0 0
#:13.704 [Sync]
.glwindow motion 423 324 0 0
.glwindow motion 423 322 0 0
#:13.735 [Sync]
.glwindow motion 424 321 0 0
#:13.766 [Sync]
#:13.797 [Sync]
.glwindow motion 424 320 0 0
#:13.829 [Sync]
.glwindow motion 423 320 0 0
.glwindow motion 421 319 0 0
#:13.86 [Sync]
.glwindow motion 420 319 0 0
.glwindow motion 419 319 0 0
#:13.891 [Sync]
.glwindow motion 418 319 0 0
#:13.922 [Sync]
.glwindow motion 416 319 0 0
.glwindow motion 415 319 0 0
#:13.954 [Sync]
.glwindow motion 413 319 0 0
.glwindow motion 412 319 0 0
#:13.985 [Sync]
.glwindow motion 411 319 0 0
.glwindow motion 410 319 0 0
#:14.016 [Sync]
.glwindow motion 410 318 0 0
#:14.047 [Sync]
.glwindow motion 409 318 0 0
#:14.079 [Sync]
#:14.11 [Sync]
#:14.141 [Sync]
.glwindow motion 408 318 0 0
#:14.172 [Sync]
#:14.204 [Sync]
.glwindow motion 407 318 0 0
#:14.235 [Sync]
.glwindow motion 406 318 0 0
.glwindow motion 406 317 0 0
#:14.266 [Sync]
.glwindow motion 405 317 0 0
.glwindow motion 404 317 0 0
#:14.297 [Sync]
.glwindow motion 403 317 0 0
#:14.329 [Sync]
#:14.36 [Sync]
#:14.391 [Sync]
.glwindow motion 402 317 0 0
#:14.422 [Sync]
#:14.454 [Sync]
#:14.485 [Sync]
#:14.516 [Sync]
#:14.547 [Sync]
#:14.579 [Sync]
.glwindow motion 401 317 0 0
#:14.61 [Sync]
#:14.641 [Sync]
#:14.672 [Sync]
#:14.704 [Sync]
.glwindow motion 400 317 0 0
#:14.735 [Sync]
#:14.766 [Sync]
#:14.797 [Sync]
#:14.829 [Sync]
.glwindow motion 399 317 0 0
.glwindow motion 399 316 0 0
#:14.86 [Sync]
#:14.891 [Sync]
.glwindow motion 398 316 0 0
#:14.922 [Sync]
#:14.954 [Sync]
.glwindow motion 398 315 0 0
#:14.985 [Sync]
#:15.016 [Sync]
#:15.047 [Sync]
#:15.079 [Sync]
#:15.11 [Sync]
.glwindow motion 398 314 0 0
#:15.141 [Sync]
.glwindow motion 397 314 0 -1
#:15.172 [Sync]
#:15.204 [Sync]
#:15.235 [Sync]
#:15.266 [Sync]
#:15.297 [Sync]
#:15.329 [Sync]
#:15.36 [Sync]
#:15.391 [Sync]
#:15.422 [Sync]
 subwindowResizeMouseStart sub0 397 351
#:15.454 [Sync]
#:15.485 [Sync]
 subwindowResizeMouseMove 1 1 sub0 396 350
.sub0.innards.togl positionUpdated 18 30 379 284
#:15.516 [Sync]
 subwindowResizeMouseMove 1 1 sub0 395 350
.sub0.innards.togl positionUpdated 18 30 378 284
#:15.547 [Sync]
 subwindowResizeMouseMove 1 1 sub0 387 344
.sub0.innards.togl positionUpdated 18 30 370 278
 subwindowResizeMouseMove 1 1 sub0 359 335
.sub0.innards.togl positionUpdated 18 30 342 269
#:15.579 [Sync]
 subwindowResizeMouseMove 1 1 sub0 323 325
.sub0.innards.togl positionUpdated 18 30 306 259
 subwindowResizeMouseMove 1 1 sub0 285 314
.sub0.innards.togl positionUpdated 18 30 268 248
#:15.61 [Sync]
 subwindowResizeMouseMove 1 1 sub0 249 288
.sub0.innards.togl positionUpdated 18 30 232 222
#:15.641 [Sync]
 subwindowResizeMouseMove 1 1 sub0 217 262
.sub0.innards.togl positionUpdated 18 30 200 196
 subwindowResizeMouseMove 1 1 sub0 206 250
.sub0.innards.togl positionUpdated 18 30 189 184
#:15.672 [Sync]
 subwindowResizeMouseMove 1 1 sub0 199 239
.sub0.innards.togl positionUpdated 18 30 182 173
 subwindowResizeMouseMove 1 1 sub0 194 232
.sub0.innards.togl positionUpdated 18 30 177 166
#:15.704 [Sync]
 subwindowResizeMouseMove 1 1 sub0 189 225
.sub0.innards.togl positionUpdated 18 30 172 159
 subwindowResizeMouseMove 1 1 sub0 184 220
.sub0.innards.togl positionUpdated 18 30 167 154
#:15.735 [Sync]
 subwindowResizeMouseMove 1 1 sub0 182 218
.sub0.innards.togl positionUpdated 18 30 165 152
#:15.766 [Sync]
 subwindowResizeMouseMove 1 1 sub0 179 216
.sub0.innards.togl positionUpdated 18 30 162 150
#:15.797 [Sync]
 subwindowResizeMouseMove 1 1 sub0 177 215
.sub0.innards.togl positionUpdated 18 30 160 149
#:15.844 [Sync]
 subwindowResizeMouseMove 1 1 sub0 175 214
.sub0.innards.togl positionUpdated 18 30 158 148
#:15.875 [Sync]
 subwindowResizeMouseMove 1 1 sub0 172 212
.sub0.innards.togl positionUpdated 18 30 155 146
#:15.907 [Sync]
 subwindowResizeMouseMove 1 1 sub0 170 211
.sub0.innards.togl positionUpdated 18 30 153 145
#:15.938 [Sync]
 subwindowResizeMouseMove 1 1 sub0 169 210
.sub0.innards.togl positionUpdated 18 30 152 144
#:15.985 [Sync]
 subwindowResizeMouseMove 1 1 sub0 168 210
.sub0.innards.togl positionUpdated 18 30 151 144
#:16.016 [Sync]
 subwindowResizeMouseMove 1 1 sub0 168 209
.sub0.innards.togl positionUpdated 18 30 151 143
#:16.047 [Sync]
 subwindowResizeMouseMove 1 1 sub0 166 208
.sub0.innards.togl positionUpdated 18 30 149 142
#:16.079 [Sync]
 subwindowResizeMouseMove 1 1 sub0 165 207
.sub0.innards.togl positionUpdated 18 30 148 141
#:16.125 [Sync]
 subwindowResizeMouseMove 1 1 sub0 164 207
.sub0.innards.togl positionUpdated 18 30 147 141
#:16.157 [Sync]
 subwindowResizeMouseMove 1 1 sub0 164 206
.sub0.innards.togl positionUpdated 18 30 147 140
#:16.188 [Sync]
#:16.219 [Sync]
 subwindowResizeMouseMove 1 1 sub0 163 206
.sub0.innards.togl positionUpdated 18 30 146 140
#:16.25 [Sync]
#:16.297 [Sync]
#:16.329 [Sync]
#:16.36 [Sync]
#:16.391 [Sync]
#:16.422 [Sync]
#:16.454 [Sync]
#:16.485 [Sync]
#:16.516 [Sync]
#:16.547 [Sync]
#:16.579 [Sync]
#:16.61 [Sync]
#:16.641 [Sync]
 subwindowResizeMouseMove 1 1 sub0 165 208
.sub0.innards.togl positionUpdated 18 30 148 142
#:16.672 [Sync]
 subwindowResizeMouseMove 1 1 sub0 179 221
.sub0.innards.togl positionUpdated 18 30 162 155
#:16.704 [Sync]
 subwindowResizeMouseMove 1 1 sub0 190 230
.sub0.innards.togl positionUpdated 18 30 173 164
 subwindowResizeMouseMove 1 1 sub0 218 241
.sub0.innards.togl positionUpdated 18 30 201 175
#:16.735 [Sync]
 subwindowResizeMouseMove 1 1 sub0 254 251
.sub0.innards.togl positionUpdated 18 30 237 185
#:16.766 [Sync]
 subwindowResizeMouseMove 1 1 sub0 294 261
.sub0.innards.togl positionUpdated 18 30 277 195
 subwindowResizeMouseMove 1 1 sub0 338 272
.sub0.innards.togl positionUpdated 18 30 321 206
#:16.797 [Sync]
 subwindowResizeMouseMove 1 1 sub0 384 284
.sub0.innards.togl positionUpdated 18 30 367 218
 subwindowResizeMouseMove 1 1 sub0 428 312
.sub0.innards.togl positionUpdated 18 30 411 246
#:16.829 [Sync]
 subwindowResizeMouseMove 1 1 sub0 474 340
.sub0.innards.togl positionUpdated 18 30 457 274
 subwindowResizeMouseMove 1 1 sub0 516 366
.sub0.innards.togl positionUpdated 18 30 499 300
#:16.86 [Sync]
 subwindowResizeMouseMove 1 1 sub0 556 378
.sub0.innards.togl positionUpdated 18 30 539 312
 subwindowResizeMouseMove 1 1 sub0 588 388
.sub0.innards.togl positionUpdated 18 30 571 322
#:16.891 [Sync]
 subwindowResizeMouseMove 1 1 sub0 618 398
.sub0.innards.togl positionUpdated 18 30 601 332
 subwindowResizeMouseMove 1 1 sub0 630 409
.sub0.innards.togl positionUpdated 18 30 613 343
#:16.922 [Sync]
 subwindowResizeMouseMove 1 1 sub0 640 420
.sub0.innards.togl positionUpdated 18 30 623 354
#:16.954 [Sync]
 subwindowResizeMouseMove 1 1 sub0 649 430
.sub0.innards.togl positionUpdated 18 30 632 364
 subwindowResizeMouseMove 1 1 sub0 658 439
.sub0.innards.togl positionUpdated 18 30 641 373
#:16.985 [Sync]
 subwindowResizeMouseMove 1 1 sub0 665 446
.sub0.innards.togl positionUpdated 18 30 648 380
 subwindowResizeMouseMove 1 1 sub0 670 453
.sub0.innards.togl positionUpdated 18 30 653 387
#:17.016 [Sync]
 subwindowResizeMouseMove 1 1 sub0 676 461
.sub0.innards.togl positionUpdated 18 30 659 395
 subwindowResizeMouseMove 1 1 sub0 678 468
.sub0.innards.togl positionUpdated 18 30 661 402
#:17.047 [Sync]
 subwindowResizeMouseMove 1 1 sub0 679 475
.sub0.innards.togl positionUpdated 18 30 662 409
 subwindowResizeMouseMove 1 1 sub0 684 481
.sub0.innards.togl positionUpdated 18 30 667 415
#:17.079 [Sync]
 subwindowResizeMouseMove 1 1 sub0 689 489
.sub0.innards.togl positionUpdated 18 30 672 423
#:17.11 [Sync]
 subwindowResizeMouseMove 1 1 sub0 695 496
.sub0.innards.togl positionUpdated 18 30 678 430
 subwindowResizeMouseMove 1 1 sub0 700 505
.sub0.innards.togl positionUpdated 18 30 683 439
#:17.141 [Sync]
 subwindowResizeMouseMove 1 1 sub0 702 513
.sub0.innards.togl positionUpdated 18 30 685 447
 subwindowResizeMouseMove 1 1 sub0 707 520
.sub0.innards.togl positionUpdated 18 30 690 454
#:17.172 [Sync]
 subwindowResizeMouseMove 1 1 sub0 712 526
.sub0.innards.togl positionUpdated 18 30 695 460
#:17.204 [Sync]
 subwindowResizeMouseMove 1 1 sub0 714 531
.sub0.innards.togl positionUpdated 18 30 697 465
 subwindowResizeMouseMove 1 1 sub0 716 532
.sub0.innards.togl positionUpdated 18 30 699 466
#:17.235 [Sync]
 subwindowResizeMouseMove 1 1 sub0 718 537
.sub0.innards.togl positionUpdated 18 30 701 471
 subwindowResizeMouseMove 1 1 sub0 719 542
.sub0.innards.togl positionUpdated 18 30 702 476
#:17.266 [Sync]
 subwindowResizeMouseMove 1 1 sub0 720 548
.sub0.innards.togl positionUpdated 18 30 703 482
 subwindowResizeMouseMove 1 1 sub0 722 553
.sub0.innards.togl positionUpdated 18 30 705 487
#:17.297 [Sync]
 subwindowResizeMouseMove 1 1 sub0 723 555
.sub0.innards.togl positionUpdated 18 30 706 489
 subwindowResizeMouseMove 1 1 sub0 725 557
.sub0.innards.togl positionUpdated 18 30 708 491
#:17.329 [Sync]
 subwindowResizeMouseMove 1 1 sub0 726 559
.sub0.innards.togl positionUpdated 18 30 709 493
 subwindowResizeMouseMove 1 1 sub0 727 564
.sub0.innards.togl positionUpdated 18 30 710 498
#:17.36 [Sync]
 subwindowResizeMouseMove 1 1 sub0 728 570
.sub0.innards.togl positionUpdated 18 30 711 504
#:17.391 [Sync]
 subwindowResizeMouseMove 1 1 sub0 729 572
.sub0.innards.togl positionUpdated 18 30 712 506
 subwindowResizeMouseMove 1 1 sub0 729 574
.sub0.innards.togl positionUpdated 18 30 712 508
#:17.422 [Sync]
 subwindowResizeMouseMove 1 1 sub0 730 575
.sub0.innards.togl positionUpdated 18 30 713 509
 subwindowResizeMouseMove 1 1 sub0 730 576
.sub0.innards.togl positionUpdated 18 30 713 510
#:17.454 [Sync]
 subwindowResizeMouseMove 1 1 sub0 731 577
.sub0.innards.togl positionUpdated 18 30 714 511
 subwindowResizeMouseMove 1 1 sub0 732 579
.sub0.innards.togl positionUpdated 18 30 715 513
#:17.485 [Sync]
 subwindowResizeMouseMove 1 1 sub0 734 581
.sub0.innards.togl positionUpdated 18 30 717 515
#:17.516 [Sync]
 subwindowResizeMouseMove 1 1 sub0 736 583
.sub0.innards.togl positionUpdated 18 30 719 517
 subwindowResizeMouseMove 1 1 sub0 741 585
.sub0.innards.togl positionUpdated 18 30 724 519
#:17.547 [Sync]
 subwindowResizeMouseMove 1 1 sub0 748 590
.sub0.innards.togl positionUpdated 18 30 731 524
 subwindowResizeMouseMove 1 1 sub0 754 595
.sub0.innards.togl positionUpdated 18 30 737 529
#:17.579 [Sync]
 subwindowResizeMouseMove 1 1 sub0 760 600
.sub0.innards.togl positionUpdated 18 30 743 534
 subwindowResizeMouseMove 1 1 sub0 762 602
.sub0.innards.togl positionUpdated 18 30 745 536
#:17.61 [Sync]
 subwindowResizeMouseMove 1 1 sub0 764 604
.sub0.innards.togl positionUpdated 18 30 747 538
 subwindowResizeMouseMove 1 1 sub0 769 609
.sub0.innards.togl positionUpdated 18 30 752 543
#:17.641 [Sync]
 subwindowResizeMouseMove 1 1 sub0 776 615
.sub0.innards.togl positionUpdated 18 30 759 549
#:17.672 [Sync]
 subwindowResizeMouseMove 1 1 sub0 782 622
.sub0.innards.togl positionUpdated 18 30 765 556
 subwindowResizeMouseMove 1 1 sub0 789 628
.sub0.innards.togl positionUpdated 18 30 772 562
#:17.704 [Sync]
 subwindowResizeMouseMove 1 1 sub0 796 635
.sub0.innards.togl positionUpdated 18 30 779 569
 subwindowResizeMouseMove 1 1 sub0 803 642
.sub0.innards.togl positionUpdated 18 30 786 576
#:17.735 [Sync]
 subwindowResizeMouseMove 1 1 sub0 810 648
.sub0.innards.togl positionUpdated 18 30 793 582
 subwindowResizeMouseMove 1 1 sub0 817 653
.sub0.innards.togl positionUpdated 18 30 800 587
#:17.766 [Sync]
 subwindowResizeMouseMove 1 1 sub0 823 659
.sub0.innards.togl positionUpdated 18 30 806 593
 subwindowResizeMouseMove 1 1 sub0 828 665
.sub0.innards.togl positionUpdated 18 30 811 599
#:17.797 [Sync]
 subwindowResizeMouseMove 1 1 sub0 834 671
.sub0.innards.togl positionUpdated 18 30 817 605
#:17.829 [Sync]
 subwindowResizeMouseMove 1 1 sub0 840 676
.sub0.innards.togl positionUpdated 18 30 823 610
 subwindowResizeMouseMove 1 1 sub0 845 681
.sub0.innards.togl positionUpdated 18 30 828 615
#:17.86 [Sync]
 subwindowResizeMouseMove 1 1 sub0 846 682
.sub0.innards.togl positionUpdated 18 30 829 616
 subwindowResizeMouseMove 1 1 sub0 848 684
.sub0.innards.togl positionUpdated 18 30 831 618
#:17.891 [Sync]
 subwindowResizeMouseMove 1 1 sub0 849 685
.sub0.innards.togl positionUpdated 18 30 832 619
#:17.922 [Sync]
 subwindowResizeMouseMove 1 1 sub0 849 686
.sub0.innards.togl positionUpdated 18 30 832 620
#:17.954 [Sync]
 subwindowResizeMouseMove 1 1 sub0 849 687
.sub0.innards.togl positionUpdated 18 30 832 621
 subwindowResizeMouseMove 1 1 sub0 850 687
.sub0.innards.togl positionUpdated 18 30 833 621
#:17.985 [Sync]
#:18.016 [Sync]
#:18.047 [Sync]
#:18.079 [Sync]
#:18.11 [Sync]
 subwindowResizeMouseMove 1 1 sub0 848 686
.sub0.innards.togl positionUpdated 18 30 831 620
#:18.141 [Sync]
 subwindowResizeMouseMove 1 1 sub0 842 679
.sub0.innards.togl positionUpdated 18 30 825 613
 subwindowResizeMouseMove 1 1 sub0 832 653
.sub0.innards.togl positionUpdated 18 30 815 587
#:18.172 [Sync]
 subwindowResizeMouseMove 1 1 sub0 821 617
.sub0.innards.togl positionUpdated 18 30 804 551
#:18.204 [Sync]
 subwindowResizeMouseMove 1 1 sub0 789 583
.sub0.innards.togl positionUpdated 18 30 772 517
 subwindowResizeMouseMove 1 1 sub0 755 557
.sub0.innards.togl positionUpdated 18 30 738 491
#:18.235 [Sync]
 subwindowResizeMouseMove 1 1 sub0 715 549
.sub0.innards.togl positionUpdated 18 30 698 483
 subwindowResizeMouseMove 1 1 sub0 679 541
.sub0.innards.togl positionUpdated 18 30 662 475
#:18.266 [Sync]
 subwindowResizeMouseMove 1 1 sub0 647 533
.sub0.innards.togl positionUpdated 18 30 630 467
 subwindowResizeMouseMove 1 1 sub0 617 526
.sub0.innards.togl positionUpdated 18 30 600 460
#:18.297 [Sync]
 subwindowResizeMouseMove 1 1 sub0 605 521
.sub0.innards.togl positionUpdated 18 30 588 455
 subwindowResizeMouseMove 1 1 sub0 596 515
.sub0.innards.togl positionUpdated 18 30 579 449
#:18.329 [Sync]
 subwindowResizeMouseMove 1 1 sub0 586 508
.sub0.innards.togl positionUpdated 18 30 569 442
#:18.36 [Sync]
 subwindowResizeMouseMove 1 1 sub0 576 499
.sub0.innards.togl positionUpdated 18 30 559 433
 subwindowResizeMouseMove 1 1 sub0 565 489
.sub0.innards.togl positionUpdated 18 30 548 423
#:18.391 [Sync]
 subwindowResizeMouseMove 1 1 sub0 539 480
.sub0.innards.togl positionUpdated 18 30 522 414
 subwindowResizeMouseMove 1 1 sub0 511 471
.sub0.innards.togl positionUpdated 18 30 494 405
#:18.422 [Sync]
 subwindowResizeMouseMove 1 1 sub0 485 464
.sub0.innards.togl positionUpdated 18 30 468 398
 subwindowResizeMouseMove 1 1 sub0 473 457
.sub0.innards.togl positionUpdated 18 30 456 391
#:18.454 [Sync]
 subwindowResizeMouseMove 1 1 sub0 462 449
.sub0.innards.togl positionUpdated 18 30 445 383
#:18.485 [Sync]
 subwindowResizeMouseMove 1 1 sub0 453 441
.sub0.innards.togl positionUpdated 18 30 436 375
 subwindowResizeMouseMove 1 1 sub0 445 433
.sub0.innards.togl positionUpdated 18 30 428 367
#:18.516 [Sync]
 subwindowResizeMouseMove 1 1 sub0 440 426
.sub0.innards.togl positionUpdated 18 30 423 360
 subwindowResizeMouseMove 1 1 sub0 434 420
.sub0.innards.togl positionUpdated 18 30 417 354
#:18.547 [Sync]
 subwindowResizeMouseMove 1 1 sub0 429 414
.sub0.innards.togl positionUpdated 18 30 412 348
 subwindowResizeMouseMove 1 1 sub0 422 408
.sub0.innards.togl positionUpdated 18 30 405 342
#:18.579 [Sync]
 subwindowResizeMouseMove 1 1 sub0 416 403
.sub0.innards.togl positionUpdated 18 30 399 337
#:18.61 [Sync]
 subwindowResizeMouseMove 1 1 sub0 410 397
.sub0.innards.togl positionUpdated 18 30 393 331
 subwindowResizeMouseMove 1 1 sub0 405 395
.sub0.innards.togl positionUpdated 18 30 388 329
 subwindowResizeMouseMove 1 1 sub0 404 393
.sub0.innards.togl positionUpdated 18 30 387 327
#:18.641 [Sync]
 subwindowResizeMouseMove 1 1 sub0 402 391
.sub0.innards.togl positionUpdated 18 30 385 325
#:18.672 [Sync]
 subwindowResizeMouseMove 1 1 sub0 397 385
.sub0.innards.togl positionUpdated 18 30 380 319
 subwindowResizeMouseMove 1 1 sub0 390 379
.sub0.innards.togl positionUpdated 18 30 373 313
#:18.704 [Sync]
 subwindowResizeMouseMove 1 1 sub0 381 372
.sub0.innards.togl positionUpdated 18 30 364 306
 subwindowResizeMouseMove 1 1 sub0 371 364
.sub0.innards.togl positionUpdated 18 30 354 298
#:18.735 [Sync]
 subwindowResizeMouseMove 1 1 sub0 363 357
.sub0.innards.togl positionUpdated 18 30 346 291
#:18.766 [Sync]
 subwindowResizeMouseMove 1 1 sub0 361 351
.sub0.innards.togl positionUpdated 18 30 344 285
 subwindowResizeMouseMove 1 1 sub0 360 349
.sub0.innards.togl positionUpdated 18 30 343 283
#:18.797 [Sync]
 subwindowResizeMouseMove 1 1 sub0 359 347
.sub0.innards.togl positionUpdated 18 30 342 281
 subwindowResizeMouseMove 1 1 sub0 358 345
.sub0.innards.togl positionUpdated 18 30 341 279
#:18.829 [Sync]
 subwindowResizeMouseMove 1 1 sub0 357 343
.sub0.innards.togl positionUpdated 18 30 340 277
 subwindowResizeMouseMove 1 1 sub0 356 341
.sub0.innards.togl positionUpdated 18 30 339 275
#:18.86 [Sync]
 subwindowResizeMouseMove 1 1 sub0 355 339
.sub0.innards.togl positionUpdated 18 30 338 273
 subwindowResizeMouseMove 1 1 sub0 354 338
.sub0.innards.togl positionUpdated 18 30 337 272
#:18.891 [Sync]
 subwindowResizeMouseMove 1 1 sub0 353 336
.sub0.innards.togl positionUpdated 18 30 336 270
 subwindowResizeMouseMove 1 1 sub0 353 334
.sub0.innards.togl positionUpdated 18 30 336 268
#:18.922 [Sync]
 subwindowResizeMouseMove 1 1 sub0 351 329
.sub0.innards.togl positionUpdated 18 30 334 263
#:18.954 [Sync]
 subwindowResizeMouseMove 1 1 sub0 351 327
.sub0.innards.togl positionUpdated 18 30 334 261
 subwindowResizeMouseMove 1 1 sub0 350 326
.sub0.innards.togl positionUpdated 18 30 333 260
#:18.985 [Sync]
#:19.016 [Sync]
#:19.047 [Sync]
 subwindowResizeMouseStop 1 1 sub0 350 326
.sub0.innards.togl positionUpdated 18 30 333 260
#:19.079 [Sync]
#:19.11 [Sync]
#:19.141 [Sync]
#:19.172 [Sync]
#:19.204 [Sync]
.sub0.innards.togl motion 319 234 0 1
.sub0.innards.togl motion 311 228 0 0
#:19.235 [Sync]
.sub0.innards.togl motion 300 221 0 0
#:19.266 [Sync]
.sub0.innards.togl motion 270 211 0 0
.sub0.innards.togl motion 226 175 0 0
#:19.297 [Sync]
.sub0.innards.togl motion 224 173 0 0
#:19.329 [Sync]
#:19.36 [Sync]
#:19.391 [Sync]
#:19.422 [Sync]
#:19.454 [Sync]
#:19.485 [Sync]
#:19.516 [Sync]
#:19.547 [Sync]
#:19.579 [Sync]
#:19.61 [Sync]
#:19.641 [Sync]
#:19.672 [Sync]
.sub0.innards.togl motion 224 172 0 0
#:19.704 [Sync]
.sub0.innards.togl motion 223 172 0 0
#:19.735 [Sync]
.sub0.innards.togl motion 221 171 0 0
.sub0.innards.togl motion 219 169 0 0
#:19.766 [Sync]
.sub0.innards.togl motion 219 168 0 0
.sub0.innards.togl motion 218 168 0 0
#:19.797 [Sync]
.sub0.innards.togl motion 218 166 0 0
#:19.829 [Sync]
.sub0.innards.togl motion 218 164 0 0
.sub0.innards.togl motion 217 162 0 0
#:19.86 [Sync]
.sub0.innards.togl motion 215 161 0 0
.sub0.innards.togl motion 209 159 0 0
#:19.891 [Sync]
.sub0.innards.togl motion 200 154 0 0
.sub0.innards.togl motion 191 146 0 0
#:19.922 [Sync]
.sub0.innards.togl motion 183 137 0 0
.sub0.innards.togl motion 178 135 0 0
#:19.954 [Sync]
.sub0.innards.togl motion 178 134 0 0
#:19.985 [Sync]
.sub0.innards.togl motion 177 134 0 0
#:20.016 [Sync]
#:20.047 [Sync]
#:20.079 [Sync]
.sub0.innards.togl motion 176 133 0 0
#:20.11 [Sync]
.sub0.innards.togl motion 175 132 0 0
#:20.141 [Sync]
.sub0.innards.togl motion 175 131 0 0
.sub0.innards.togl motion 174 130 0 0
#:20.172 [Sync]
.sub0.innards.togl motion 174 129 0 0
.sub0.innards.togl motion 173 127 0 0
#:20.204 [Sync]
.sub0.innards.togl motion 171 126 0 0
.sub0.innards.togl motion 170 125 0 0
#:20.235 [Sync]
.sub0.innards.togl motion 169 124 0 0
#:20.266 [Sync]
.sub0.innards.togl motion 168 124 0 0
#:20.297 [Sync]
#:20.329 [Sync]
#:20.36 [Sync]
.sub0.innards.togl motion 167 124 0 0
#:20.391 [Sync]
#:20.422 [Sync]
.sub0.innards.togl motion 166 124 0 0
#:20.454 [Sync]
.sub0.innards.togl motion 166 123 0 0
.sub0.innards.togl motion 165 123 0 0
#:20.485 [Sync]
.sub0.innards.togl motion 165 122 0 0
#:20.516 [Sync]
#:20.547 [Sync]
#:20.579 [Sync]
.glwindow posable set Window.sub0 xyz 9623.758789 15200.157227 538.589661
#:20.61 [Sync]
.glwindow posable set Window.sub0 xyz 9629.791992 15178.136719 535.544739
#:20.641 [Sync]
.glwindow posable set Window.sub0 xyz 9634.316406 15161.621094 533.261047
#:20.688 [Sync]
.sub0.innards.togl motion 164 122 512 0
.sub0.innards.togl motion 164 121 512 0
.glwindow posable set Window.sub0 xyz 9637.709961 15149.234375 531.548279
#:20.719 [Sync]
.sub0.innards.togl motion 162 120 512 0
.sub0.innards.togl motion 160 119 512 0
.glwindow posable set Window.sub0 xyz 9640.254883 15139.944336 530.263672
#:20.75 [Sync]
.sub0.innards.togl motion 155 114 512 0
.sub0.innards.togl motion 150 108 512 0
.glwindow posable set Window.sub0 s 0.405090
.glwindow posable set Window.sub0 xyz 9642.164063 15132.976563 529.300232
#:20.782 [Sync]
.sub0.innards.togl motion 148 102 512 0
.sub0.innards.togl motion 147 97 512 0
.glwindow posable set Window.sub0 s 0.437497
.glwindow posable set Window.sub0 xyz 9643.595703 15127.750977 528.577637
#:20.829 [Sync]
.sub0.innards.togl motion 147 91 512 0
.sub0.innards.togl motion 146 86 512 0
.glwindow posable set Window.sub0 s 0.472497
.glwindow posable set Window.sub0 xyz 9644.669922 15123.832031 528.035706
#:20.86 [Sync]
.sub0.innards.togl motion 145 83 512 0
.glwindow posable set Window.sub0 s 0.510297
.glwindow posable set Window.sub0 xyz 9645.475586 15120.892578 527.629272
#:20.891 [Sync]
.sub0.innards.togl motion 144 81 512 0
.glwindow posable set Window.sub0 s 0.551121
.glwindow posable set Window.sub0 xyz 9646.079102 15118.688477 527.324463
#:20.938 [Sync]
.sub0.innards.togl motion 144 79 512 0
.glwindow posable set Window.sub0 s 0.595210
.glwindow posable set Window.sub0 xyz 9646.532227 15117.035156 527.095825
#:20.969 [Sync]
.sub0.innards.togl motion 143 77 512 0
.glwindow posable set Window.sub0 s 0.642827
.glwindow posable set Window.sub0 xyz 9646.872070 15115.794922 526.924377
#:21 [Sync]
.glwindow posable set Window.sub0 s 0.694253
.glwindow posable set Window.sub0 xyz 9647.126953 15114.865234 526.795776
#:21.032 [Sync]
.sub0.innards.togl motion 143 76 512 0
.glwindow posable set Window.sub0 s 0.749793
.glwindow posable set Window.sub0 xyz 9647.318359 15114.167969 526.699341
#:21.079 [Sync]
.glwindow posable set Window.sub0 s 0.809777
.glwindow posable set Window.sub0 xyz 9647.461914 15113.644531 526.627014
#:21.11 [Sync]
.sub0.innards.togl motion 143 75 512 0
.glwindow posable set Window.sub0 s 0.874559
.glwindow posable set Window.sub0 xyz 9647.569336 15113.251953 526.572754
#:21.141 [Sync]
.glwindow posable set Window.sub0 s 0.944524
.glwindow posable set Window.sub0 xyz 9647.650391 15112.958008 526.532043
#:21.172 [Sync]
.glwindow posable set Window.sub0 s 1.020086
.glwindow posable set Window.sub0 xyz 9647.710938 15112.737305 526.501526
#:21.219 [Sync]
.glwindow posable set Window.sub0 s 1.101693
.glwindow posable set Window.sub0 xyz 9647.755859 15112.571289 526.478638
#:21.25 [Sync]
.glwindow posable set Window.sub0 s 1.189828
.glwindow posable set Window.sub0 xyz 9647.790039 15112.447266 526.461487
#:21.282 [Sync]
.glwindow posable set Window.sub0 s 1.285015
.glwindow posable set Window.sub0 xyz 9647.815430 15112.354492 526.448608
#:21.313 [Sync]
.glwindow posable set Window.sub0 s 1.387816
.glwindow posable set Window.sub0 xyz 9647.834961 15112.284180 526.438965
#:21.36 [Sync]
.glwindow posable set Window.sub0 s 1.498841
.glwindow posable set Window.sub0 xyz 9647.849609 15112.231445 526.431702
#:21.391 [Sync]
.glwindow posable set Window.sub0 s 1.618749
.glwindow posable set Window.sub0 xyz 9647.860352 15112.192383 526.426270
#:21.422 [Sync]
.glwindow posable set Window.sub0 s 1.748248
.glwindow posable set Window.sub0 xyz 9647.868164 15112.163086 526.422180
#:21.454 [Sync]
.glwindow posable set Window.sub0 s 1.888108
.glwindow posable set Window.sub0 xyz 9647.874023 15112.140625 526.419128
#:21.5 [Sync]
.glwindow posable set Window.sub0 s 2.039157
.glwindow posable set Window.sub0 xyz 9647.878906 15112.124023 526.416809
#:21.532 [Sync]
.glwindow posable set Window.sub0 s 2.202290
.glwindow posable set Window.sub0 xyz 9647.881836 15112.111328 526.415100
#:21.563 [Sync]
.glwindow posable set Window.sub0 s 2.378473
.glwindow posable set Window.sub0 xyz 9647.884766 15112.101563 526.413818
#:21.594 [Sync]
.glwindow posable set Window.sub0 s 2.568751
.glwindow posable set Window.sub0 xyz 9647.886719 15112.094727 526.412842
#:21.641 [Sync]
.glwindow posable set Window.sub0 s 2.774251
.glwindow posable set Window.sub0 xyz 9647.887695 15112.089844 526.412109
#:21.688 [Sync]
.glwindow posable set Window.sub0 s 2.996192
.glwindow posable set Window.sub0 xyz 9647.888672 15112.085938 526.411560
#:21.735 [Sync]
.glwindow posable set Window.sub0 s 3.235887
.glwindow posable set Window.sub0 xyz 9647.889648 15112.083008 526.411133
#:21.782 [Sync]
.glwindow posable set Window.sub0 s 3.494758
.glwindow posable set Window.sub0 xyz 9647.890625 15112.081055 526.410828
#:21.829 [Sync]
.glwindow posable set Window.sub0 s 3.774339
.glwindow posable set Window.sub0 xyz 9647.890625 15112.079102 526.410583
#:21.891 [Sync]
.glwindow posable set Window.sub0 s 4.076286
.glwindow posable set Window.sub0 xyz 9647.890625 15112.078125 526.410400
#:21.938 [Sync]
.glwindow posable set Window.sub0 s 4.402390
.glwindow posable set Window.sub0 xyz 9647.890625 15112.077148 526.410278
#:21.985 [Sync]
.sub0.innards.togl motion 142 75 512 0
.sub0.innards.togl motion 141 77 0 0
#:22.016 [Sync]
.sub0.innards.togl motion 141 82 0 0
#:22.047 [Sync]
.sub0.innards.togl motion 141 87 0 0
.sub0.innards.togl motion 142 92 0 0
#:22.079 [Sync]
.sub0.innards.togl motion 142 97 0 0
.sub0.innards.togl motion 142 98 0 0
#:22.11 [Sync]
.sub0.innards.togl motion 142 100 0 0
.sub0.innards.togl motion 142 101 0 0
#:22.141 [Sync]
.sub0.innards.togl motion 142 102 0 0
.sub0.innards.togl motion 142 103 0 0
#:22.172 [Sync]
#:22.204 [Sync]
#:22.235 [Sync]
#:22.266 [Sync]
#:22.297 [Sync]
#:22.329 [Sync]
#:22.36 [Sync]
#:22.391 [Sync]
#:22.422 [Sync]
#:22.454 [Sync]
.sub0.innards.togl keyboard 142 103 0 f 0 102
#:22.61 [Sync]
.sub0.innards.togl motion 143 103 0 0
.sub0.innards.togl keyboard 143 103 0 f 1 102
#:22.641 [Sync]
.sub0.innards.togl motion 144 103 0 0
#:22.672 [Sync]
#:22.704 [Sync]
.sub0.innards.togl motion 144 104 0 0
#:22.735 [Sync]
#:22.766 [Sync]
.sub0.innards.togl motion 143 104 0 0
.sub0.innards.togl motion 142 104 0 0
#:22.797 [Sync]
.sub0.innards.togl motion 141 104 0 0
.sub0.innards.togl motion 140 104 0 0
#:22.829 [Sync]
.sub0.innards.togl motion 139 104 0 0
#:22.86 [Sync]
.sub0.innards.togl motion 138 104 0 0
.sub0.innards.togl motion 137 104 0 0
#:22.891 [Sync]
.sub0.innards.togl motion 136 104 0 0
.sub0.innards.togl motion 135 104 0 0
#:22.922 [Sync]
.sub0.innards.togl motion 134 104 0 0
.sub0.innards.togl motion 133 104 0 0
#:22.954 [Sync]
#:22.985 [Sync]
#:23.016 [Sync]
#:23.079 [Sync]
.glwindow posable set Window.sub0 xyz 9636.810547 15120.596680 528.142822
#:23.125 [Sync]
.glwindow posable set Window.sub0 xyz 9628.500977 15126.986328 529.442261
#:23.172 [Sync]
.sub0.innards.togl motion 134 104 512 0
.glwindow posable set Window.sub0 xyz 9622.268555 15131.778320 530.416809
#:23.219 [Sync]
.sub0.innards.togl motion 135 97 512 0
.sub0.innards.togl motion 135 96 512 0
.glwindow posable set Window.sub0 s 4.754581
.glwindow posable set Window.sub0 xyz 9617.594727 15135.372070 531.147705
#:23.266 [Sync]
.sub0.innards.togl motion 135 92 512 0
.glwindow posable set Window.sub0 s 5.134948
.glwindow posable set Window.sub0 xyz 9614.088867 15138.067383 531.695923
#:23.329 [Sync]
.sub0.innards.togl motion 135 87 512 0
.glwindow posable set Window.sub0 s 5.545744
.glwindow posable set Window.sub0 xyz 9611.459961 15140.088867 532.107056
#:23.407 [Sync]
.sub0.innards.togl motion 135 84 512 0
.glwindow posable set Window.sub0 s 5.989404
.glwindow posable set Window.sub0 xyz 9609.488281 15141.605469 532.415405
#:23.469 [Sync]
.sub0.innards.togl motion 135 83 512 0
.glwindow posable set Window.sub0 s 6.468556
.glwindow posable set Window.sub0 xyz 9608.009766 15142.742188 532.646667
#:23.547 [Sync]
.glwindow posable set Window.sub0 s 6.986041
.glwindow posable set Window.sub0 xyz 9606.900391 15143.594727 532.820129
#:23.61 [Sync]
.sub0.innards.togl motion 135 82 512 0
.glwindow posable set Window.sub0 s 7.544925
.glwindow posable set Window.sub0 xyz 9606.068359 15144.234375 532.950195
#:23.688 [Sync]
.glwindow posable set Window.sub0 s 8.148519
.glwindow posable set Window.sub0 xyz 9605.444336 15144.713867 533.047791
#:23.75 [Sync]
.glwindow posable set Window.sub0 s 8.800401
.glwindow posable set Window.sub0 xyz 9604.976563 15145.073242 533.120972
#:23.829 [Sync]
.glwindow posable set Window.sub0 s 9.504434
.glwindow posable set Window.sub0 xyz 9604.625977 15145.342773 533.175842
#:23.891 [Sync]
.glwindow posable set Window.sub0 s 10.264789
.glwindow posable set Window.sub0 xyz 9604.362305 15145.544922 533.216980
#:23.954 [Sync]
.glwindow posable set Window.sub0 s 11.085972
.glwindow posable set Window.sub0 xyz 9604.165039 15145.696289 533.247864
#:24.016 [Sync]
.glwindow posable set Window.sub0 s 11.972850
.glwindow posable set Window.sub0 xyz 9604.016602 15145.810547 533.270996
#:24.079 [Sync]
.glwindow posable set Window.sub0 s 12.930678
.glwindow posable set Window.sub0 xyz 9603.905273 15145.895508 533.288391
#:24.125 [Sync]
.glwindow posable set Window.sub0 s 13.965134
.glwindow posable set Window.sub0 xyz 9603.822266 15145.959961 533.301392
#:24.188 [Sync]
.glwindow posable set Window.sub0 s 15.082345
.glwindow posable set Window.sub0 xyz 9603.759766 15146.007813 533.311157
#:24.235 [Sync]
.glwindow posable set Window.sub0 s 16.288933
.glwindow posable set Window.sub0 xyz 9603.712891 15146.043945 533.318481
#:24.282 [Sync]
.glwindow posable set Window.sub0 s 17.592049
.glwindow posable set Window.sub0 xyz 9603.677734 15146.071289 533.323975
#:24.329 [Sync]
.glwindow posable set Window.sub0 s 18.999413
.glwindow posable set Window.sub0 xyz 9603.651367 15146.091797 533.328125
#:24.375 [Sync]
.glwindow posable set Window.sub0 s 20.519367
.glwindow posable set Window.sub0 xyz 9603.631836 15146.107422 533.331238
#:24.422 [Sync]
.glwindow posable set Window.sub0 s 22.160917
.glwindow posable set Window.sub0 xyz 9603.617188 15146.119141 533.333557
#:24.469 [Sync]
.glwindow posable set Window.sub0 s 23.933792
.glwindow posable set Window.sub0 xyz 9603.606445 15146.127930 533.335266
#:24.516 [Sync]
.glwindow posable set Window.sub0 s 25.848497
.glwindow posable set Window.sub0 xyz 9603.597656 15146.134766 533.336548
#:24.563 [Sync]
#:24.594 [Sync]
#:24.625 [Sync]
.sub0.innards.togl motion 135 81 0 0
#:24.657 [Sync]
.sub0.innards.togl motion 135 80 0 0
#:24.688 [Sync]
#:24.719 [Sync]
.sub0.innards.togl motion 135 79 0 0
#:24.75 [Sync]
.sub0.innards.togl motion 135 78 0 0
.sub0.innards.togl motion 135 77 0 0
#:24.782 [Sync]
.sub0.innards.togl motion 135 76 0 0
.sub0.innards.togl motion 135 75 0 0
#:24.813 [Sync]
.sub0.innards.togl motion 135 74 0 0
#:24.844 [Sync]
#:24.875 [Sync]
#:24.907 [Sync]
#:24.938 [Sync]
#:24.969 [Sync]
.sub0.innards.togl motion 136 74 0 0
#:25 [Sync]
.sub0.innards.togl motion 137 73 0 0
#:25.032 [Sync]
.sub0.innards.togl motion 138 73 0 0
.sub0.innards.togl motion 139 72 0 0
#:25.063 [Sync]
.sub0.innards.togl motion 141 72 0 0
#:25.094 [Sync]
.sub0.innards.togl motion 142 71 0 0
#:25.125 [Sync]
#:25.157 [Sync]
#:25.188 [Sync]
#:25.219 [Sync]
#:25.266 [Sync]
.glwindow posable set Window.sub0 xyz 9600.903320 15170.030273 528.848633
#:25.297 [Sync]
.sub0.innards.togl motion 143 71 512 0
.glwindow posable set Window.sub0 xyz 9598.881836 15187.952148 525.482727
#:25.344 [Sync]
.glwindow posable set Window.sub0 xyz 9597.366211 15201.393555 522.958313
#:25.391 [Sync]
.glwindow posable set Window.sub0 xyz 9596.229492 15211.474609 521.065002
#:25.438 [Sync]
.glwindow posable set Window.sub0 xyz 9595.376953 15219.035156 519.645020
#:25.485 [Sync]
.glwindow posable set Window.sub0 xyz 9594.737305 15224.706055 518.580017
#:25.532 [Sync]
.glwindow posable set Window.sub0 xyz 9594.257813 15228.958984 517.781250
#:25.579 [Sync]
.glwindow posable set Window.sub0 xyz 9593.898438 15232.148438 517.182190
#:25.625 [Sync]
.glwindow posable set Window.sub0 xyz 9593.628906 15234.541016 516.732910
#:25.672 [Sync]
.glwindow posable set Window.sub0 xyz 9593.426758 15236.334961 516.395935
#:25.719 [Sync]
.glwindow posable set Window.sub0 xyz 9593.275391 15237.680664 516.143188
#:25.766 [Sync]
#:25.797 [Sync]
#:25.829 [Sync]
.sub0.innards.togl motion 145 71 0 0
.sub0.innards.togl motion 153 70 0 0
#:25.86 [Sync]
.sub0.innards.togl motion 165 69 0 0
.sub0.innards.togl motion 177 68 0 0
#:25.891 [Sync]
.sub0.innards.togl motion 187 67 0 0
.sub0.innards.togl motion 195 65 0 0
#:25.922 [Sync]
.sub0.innards.togl motion 201 64 0 0
#:25.954 [Sync]
.sub0.innards.togl motion 202 63 0 0
.sub0.innards.togl motion 202 62 0 0
#:25.985 [Sync]
.sub0.innards.togl motion 202 61 0 0
#:26.016 [Sync]
.sub0.innards.togl motion 202 60 0 0
#:26.047 [Sync]
.sub0.innards.togl motion 202 59 0 0
#:26.079 [Sync]
.sub0.innards.togl motion 202 58 0 0
#:26.11 [Sync]
#:26.141 [Sync]
#:26.172 [Sync]
.sub0.innards.togl motion 203 58 0 0
.sub0.innards.togl motion 204 58 0 0
#:26.204 [Sync]
.sub0.innards.togl motion 206 58 0 0
#:26.235 [Sync]
.sub0.innards.togl motion 208 58 0 0
.sub0.innards.togl motion 210 59 0 0
#:26.266 [Sync]
.sub0.innards.togl motion 211 59 0 0
#:26.297 [Sync]
.sub0.innards.togl motion 211 60 0 0
#:26.329 [Sync]
#:26.36 [Sync]
#:26.391 [Sync]
#:26.422 [Sync]
#:26.454 [Sync]
#:26.485 [Sync]
#:26.516 [Sync]
.sub0.innards.togl motion 212 61 0 0
#:26.563 [Sync]
.glwindow posable set Window.sub0 xyz 9597.445313 15247.683594 516.830322
#:26.61 [Sync]
.sub0.innards.togl motion 212 60 512 0
.glwindow posable set Window.sub0 xyz 9600.572266 15255.185547 517.345703
#:26.657 [Sync]
.sub0.innards.togl motion 212 53 512 0
.sub0.innards.togl motion 212 51 512 0
.glwindow posable set Window.sub0 s 27.916378
.glwindow posable set Window.sub0 xyz 9602.917969 15260.812500 517.732239
#:26.688 [Sync]
.sub0.innards.togl motion 211 44 512 0
.glwindow posable set Window.sub0 s 30.149689
.glwindow posable set Window.sub0 xyz 9604.676758 15265.032227 518.022156
#:26.75 [Sync]
.sub0.innards.togl motion 208 41 512 0
.sub0.innards.togl motion 207 40 512 0
.glwindow posable set Window.sub0 s 32.561665
.glwindow posable set Window.sub0 xyz 9605.996094 15268.197266 518.239563
#:26.797 [Sync]
.sub0.innards.togl motion 206 39 512 0
.sub0.innards.togl motion 206 38 512 0
.glwindow posable set Window.sub0 s 35.166599
.glwindow posable set Window.sub0 xyz 9606.985352 15270.571289 518.402649
#:26.844 [Sync]
.glwindow posable set Window.sub0 s 37.979927
.glwindow posable set Window.sub0 xyz 9607.727539 15272.351563 518.524963
#:26.891 [Sync]
.glwindow posable set Window.sub0 s 41.018322
.glwindow posable set Window.sub0 xyz 9608.284180 15273.686523 518.616699
#:26.922 [Sync]
.glwindow posable set Window.sub0 s 44.299789
.glwindow posable set Window.sub0 xyz 9608.701172 15274.687500 518.685486
#:26.954 [Sync]
.glwindow posable set Window.sub0 s 47.843773
.glwindow posable set Window.sub0 xyz 9609.014648 15275.438477 518.737061
#:26.985 [Sync]
.glwindow posable set Window.sub0 s 51.671276
.glwindow posable set Window.sub0 xyz 9609.249023 15276.001953 518.775757
#:27.032 [Sync]
.glwindow posable set Window.sub0 s 55.804981
.glwindow posable set Window.sub0 xyz 9609.424805 15276.424805 518.804810
#:27.063 [Sync]
.glwindow posable set Window.sub0 s 60.269382
.glwindow posable set Window.sub0 xyz 9609.556641 15276.741211 518.826599
#:27.094 [Sync]
.glwindow posable set Window.sub0 s 65.090935
.glwindow posable set Window.sub0 xyz 9609.655273 15276.978516 518.842896
#:27.125 [Sync]
.glwindow posable set Window.sub0 s 70.298210
.glwindow posable set Window.sub0 xyz 9609.729492 15277.157227 518.855164
#:27.172 [Sync]
.glwindow posable set Window.sub0 s 75.922073
.glwindow posable set Window.sub0 xyz 9609.785156 15277.291016 518.864319
#:27.204 [Sync]
.glwindow posable set Window.sub0 s 81.995842
.glwindow posable set Window.sub0 xyz 9609.827148 15277.391602 518.871216
#:27.235 [Sync]
.glwindow posable set Window.sub0 s 88.555511
.glwindow posable set Window.sub0 xyz 9609.858398 15277.466797 518.876404
#:27.266 [Sync]
.glwindow posable set Window.sub0 s 95.639954
.glwindow posable set Window.sub0 xyz 9609.881836 15277.523438 518.880249
#:27.313 [Sync]
.glwindow posable set Window.sub0 s 103.291153
.glwindow posable set Window.sub0 xyz 9609.899414 15277.565430 518.883179
#:27.344 [Sync]
.glwindow posable set Window.sub0 s 111.554451
.glwindow posable set Window.sub0 xyz 9609.913086 15277.596680 518.885376
#:27.375 [Sync]
.glwindow posable set Window.sub0 s 120.478813
.glwindow posable set Window.sub0 xyz 9609.922852 15277.620117 518.887024
#:27.407 [Sync]
.glwindow posable set Window.sub0 s 130.117126
.glwindow posable set Window.sub0 xyz 9609.930664 15277.637695 518.888245
#:27.454 [Sync]
.glwindow posable set Window.sub0 s 140.526505
.glwindow posable set Window.sub0 xyz 9609.936523 15277.651367 518.889160
#:27.485 [Sync]
.glwindow posable set Window.sub0 s 151.768631
.glwindow posable set Window.sub0 xyz 9609.940430 15277.661133 518.889832
#:27.516 [Sync]
.glwindow posable set Window.sub0 s 163.910126
.glwindow posable set Window.sub0 xyz 9609.943359 15277.668945 518.890320
#:27.563 [Sync]
.glwindow posable set Window.sub0 s 177.022949
.glwindow posable set Window.sub0 xyz 9609.945313 15277.674805 518.890686
#:27.594 [Sync]
.sub0.innards.togl motion 205 37 512 0
.glwindow posable set Window.sub0 s 191.184799
.glwindow posable set Window.sub0 xyz 9609.947266 15277.678711 518.890991
#:27.641 [Sync]
.glwindow posable set Window.sub0 s 206.479584
.glwindow posable set Window.sub0 xyz 9609.948242 15277.681641 518.891235
#:27.672 [Sync]
.glwindow posable set Window.sub0 s 222.997955
.glwindow posable set Window.sub0 xyz 9609.949219 15277.683594 518.891418
#:27.704 [Sync]
.glwindow posable set Window.sub0 s 240.837799
.glwindow posable set Window.sub0 xyz 9609.950195 15277.685547 518.891541
#:27.735 [Sync]
.glwindow posable set Window.sub0 s 260.104828
.glwindow posable set Window.sub0 xyz 9609.951172 15277.686523 518.891663
#:27.782 [Sync]
.glwindow posable set Window.sub0 s 280.913239
.glwindow posable set Window.sub0 xyz 9609.951172 15277.687500 518.891724
#:27.813 [Sync]
.glwindow posable set Window.sub0 s 303.386322
.glwindow posable set Window.sub0 xyz 9609.951172 15277.688477 518.891785
#:27.844 [Sync]
.sub0.innards.togl motion 204 37 512 0
.glwindow posable set Window.sub0 s 327.657227
.glwindow posable set Window.sub0 xyz 9609.951172 15277.689453 518.891846
#:27.891 [Sync]
.sub0.innards.togl motion 204 36 512 0
.glwindow posable set Window.sub0 s 353.869812
.glwindow posable set Window.sub0 xyz 9609.951172 15277.689453 518.891846
#:27.922 [Sync]
.sub0.innards.togl motion 203 36 512 0
.sub0.innards.togl motion 201 34 512 0
.sub0.innards.togl motion 200 34 0 0
#:27.954 [Sync]
#:27.985 [Sync]
#:28.016 [Sync]
.sub0.innards.togl motion 199 34 0 0
.sub0.innards.togl motion 197 35 0 0
#:28.047 [Sync]
.sub0.innards.togl motion 195 36 0 0
.sub0.innards.togl motion 193 38 0 0
#:28.079 [Sync]
.sub0.innards.togl motion 191 43 0 0
#:28.11 [Sync]
.sub0.innards.togl motion 190 48 0 0
.sub0.innards.togl motion 188 55 0 0
#:28.141 [Sync]
.sub0.innards.togl motion 186 65 0 0
.sub0.innards.togl motion 184 74 0 0
#:28.172 [Sync]
.sub0.innards.togl motion 178 82 0 0
.sub0.innards.togl motion 173 88 0 0
#:28.204 [Sync]
.sub0.innards.togl motion 167 90 0 0
#:28.235 [Sync]
.sub0.innards.togl motion 162 92 0 0
.sub0.innards.togl motion 160 92 0 0
#:28.266 [Sync]
#:28.297 [Sync]
.sub0.innards.togl motion 159 92 0 0
.sub0.innards.togl motion 158 92 0 0
#:28.329 [Sync]
.sub0.innards.togl motion 157 92 0 0
.sub0.innards.togl motion 156 92 0 0
#:28.36 [Sync]
.sub0.innards.togl motion 154 92 0 0
#:28.391 [Sync]
.sub0.innards.togl motion 153 91 0 0
#:28.422 [Sync]
.sub0.innards.togl motion 151 91 0 0
.sub0.innards.togl motion 150 90 0 0
#:28.454 [Sync]
#:28.485 [Sync]
.sub0.innards.togl motion 149 90 0 0
#:28.516 [Sync]
.sub0.innards.togl mouse 149 90 0 1 0
#:28.547 [Sync]
#:28.579 [Sync]
.sub0.innards.togl mouse 149 90 256 1 1
#:29.485 [Sync]
.sub0.innards.togl motion 164 47 0 0
#:29.516 [Sync]
#:29.547 [Sync]
#:29.579 [Sync]
#:29.61 [Sync]
.sub0.innards.togl motion 163 47 0 0
#:29.641 [Sync]
#:29.672 [Sync]
#:29.704 [Sync]
#:29.735 [Sync]
#:29.766 [Sync]
.sub0.innards.togl mouse 163 47 0 1 0
#:29.797 [Sync]
#:29.844 [Sync]
#:29.875 [Sync]
#:29.907 [Sync]
.sub0.innards.togl motion 163 46 256 0
.glwindow posable set Window.sub0 p 24.500000
#:29.938 [Sync]
#:29.969 [Sync]
.sub0.innards.togl motion 163 45 256 0
.sub0.innards.togl motion 163 44 256 0
.glwindow posable set Window.sub0 p 23.500000
#:30 [Sync]
.sub0.innards.togl motion 162 43 256 0
.glwindow posable set Window.sub0 p 23.000000
#:30.032 [Sync]
#:30.079 [Sync]
#:30.11 [Sync]
.sub0.innards.togl motion 162 42 256 0
.glwindow posable set Window.sub0 p 22.500000
#:30.141 [Sync]
.sub0.innards.togl motion 162 40 256 0
.glwindow posable set Window.sub0 p 21.500000
#:30.172 [Sync]
.sub0.innards.togl motion 162 38 256 0
.glwindow posable set Window.sub0 p 20.500000
#:30.219 [Sync]
.sub0.innards.togl motion 162 37 256 0
.glwindow posable set Window.sub0 p 20.000000
#:30.25 [Sync]
.sub0.innards.togl motion 162 36 256 0
.glwindow posable set Window.sub0 p 19.500000
#:30.282 [Sync]
.sub0.innards.togl motion 162 35 256 0
.glwindow posable set Window.sub0 p 19.000000
#:30.313 [Sync]
.sub0.innards.togl motion 162 34 256 0
.glwindow posable set Window.sub0 p 18.500000
#:30.36 [Sync]
.sub0.innards.togl motion 161 33 256 0
.glwindow posable set Window.sub0 p 18.000000
#:30.391 [Sync]
.sub0.innards.togl motion 161 31 256 0
.glwindow posable set Window.sub0 p 17.000000
#:30.422 [Sync]
.sub0.innards.togl motion 161 29 256 0
.glwindow posable set Window.sub0 p 16.000000
#:30.454 [Sync]
.sub0.innards.togl motion 161 27 256 0
.glwindow posable set Window.sub0 p 15.000000
#:30.5 [Sync]
.sub0.innards.togl motion 161 26 256 0
.glwindow posable set Window.sub0 p 14.500000
#:30.532 [Sync]
.sub0.innards.togl motion 161 25 256 0
.glwindow posable set Window.sub0 p 14.000000
#:30.563 [Sync]
.sub0.innards.togl motion 161 24 256 0
.glwindow posable set Window.sub0 p 13.500000
#:30.594 [Sync]
.sub0.innards.togl motion 161 23 256 0
.glwindow posable set Window.sub0 p 13.000000
#:30.641 [Sync]
.sub0.innards.togl motion 161 22 256 0
.glwindow posable set Window.sub0 p 12.500000
#:30.672 [Sync]
.sub0.innards.togl motion 161 21 256 0
.glwindow posable set Window.sub0 p 12.000000
#:30.704 [Sync]
.sub0.innards.togl motion 161 20 256 0
.glwindow posable set Window.sub0 p 11.500000
#:30.75 [Sync]
.sub0.innards.togl motion 161 19 256 0
.glwindow posable set Window.sub0 p 11.000000
#:30.782 [Sync]
.sub0.innards.togl motion 161 18 256 0
.glwindow posable set Window.sub0 p 10.500000
#:30.813 [Sync]
.sub0.innards.togl motion 160 16 256 0
.glwindow posable set Window.sub0 p 9.500000
#:30.844 [Sync]
.sub0.innards.togl motion 159 14 256 0
.glwindow posable set Window.sub0 p 8.500000
.sub0.innards.togl mouse 159 14 256 1 1
#:30.891 [Sync]
#:30.922 [Sync]
#:30.954 [Sync]
#:30.985 [Sync]
.sub0.innards.togl motion 161 16 0 0
.sub0.innards.togl motion 166 21 0 0
#:31.016 [Sync]
.sub0.innards.togl motion 175 26 0 0
.sub0.innards.togl motion 209 33 0 0
#:31.047 [Sync]
.sub0.innards.togl motion 257 43 0 0
.sub0.innards.togl motion 323 53 0 0
#:31.079 [Sync]
.sub0.innards.togl motion 401 60 0 -1
.glwindow motion 422 112 0 1
#:31.11 [Sync]
.glwindow motion 502 117 0 0
.glwindow motion 566 118 0 0
#:31.141 [Sync]
.glwindow motion 606 117 0 0
.glwindow motion 607 117 0 0
#:31.172 [Sync]
#:31.204 [Sync]
#:31.235 [Sync]
#:31.266 [Sync]
#:31.297 [Sync]
#:31.329 [Sync]
#:31.36 [Sync]
#:31.391 [Sync]
.glwindow motion 608 116 0 0
#:31.422 [Sync]
.glwindow motion 640 116 0 0
#:31.454 [Sync]
.glwindow motion 670 117 0 0
.glwindow motion 702 124 0 0
#:31.485 [Sync]
.glwindow motion 730 133 0 0
.glwindow motion 740 141 0 0
#:31.516 [Sync]
.glwindow motion 745 149 0 0
.glwindow motion 747 156 0 0
#:31.547 [Sync]
.glwindow motion 747 161 0 0
.glwindow motion 747 162 0 0
#:31.579 [Sync]
#:31.61 [Sync]
.glwindow motion 747 163 0 0
#:31.641 [Sync]
.glwindow motion 747 164 0 0
.glwindow motion 746 164 0 0
#:31.672 [Sync]
.glwindow motion 745 164 0 0
.glwindow motion 744 164 0 0
#:31.704 [Sync]
.glwindow motion 739 163 0 0
.glwindow motion 732 161 0 0
#:31.735 [Sync]
.glwindow motion 725 160 0 0
#:31.766 [Sync]
.glwindow motion 719 159 0 0
.glwindow motion 717 159 0 0
#:31.797 [Sync]
.glwindow motion 715 158 0 0
#:31.829 [Sync]
.glwindow motion 713 157 0 0
.glwindow motion 712 156 0 0
#:31.86 [Sync]
.glwindow motion 711 156 0 0
#:31.891 [Sync]
.glwindow motion 710 156 0 0
.glwindow motion 709 156 0 0
#:31.922 [Sync]
.glwindow motion 707 156 0 0
.glwindow motion 706 155 0 0
#:31.954 [Sync]
.glwindow motion 704 154 0 0
.glwindow motion 703 153 0 0
#:31.985 [Sync]
.glwindow motion 702 153 0 0
#:32.016 [Sync]
#:32.047 [Sync]
.glwindow motion 701 153 0 0
#:32.079 [Sync]
#:32.11 [Sync]
#:32.141 [Sync]
.glwindow motion 700 153 0 0
#:32.172 [Sync]
#:32.204 [Sync]
.glwindow motion 700 154 0 0
#:32.235 [Sync]
.glwindow motion 699 154 0 0
#:32.266 [Sync]
#:32.297 [Sync]
#:32.329 [Sync]
#:32.36 [Sync]
.glwindow motion 698 154 0 0
#:32.391 [Sync]
.glwindow posable set Window.main xyz 6243.630859 6497.520996 333.853088
.glwindow posable set Window.main xyz 7096.316895 8666.115234 389.573090
#:32.438 [Sync]
.glwindow motion 697 154 512 0
.glwindow posable set Window.main xyz 7735.831543 10292.560547 431.363098
#:32.469 [Sync]
.glwindow motion 696 154 512 0
.glwindow posable set Window.main xyz 8215.467773 11512.394531 462.705597
#:32.5 [Sync]
.glwindow motion 691 149 512 0
.glwindow posable set Window.main s 0.145993
.glwindow posable set Window.main xyz 8575.194336 12427.270508 486.212463
#:32.532 [Sync]
.glwindow motion 681 130 512 0
.glwindow posable set Window.main s 0.157672
.glwindow posable set Window.main xyz 8844.989258 13113.427734 503.842621
#:32.563 [Sync]
.glwindow motion 668 113 512 0
.glwindow posable set Window.main s 0.170286
.glwindow posable set Window.main xyz 9047.335938 13628.044922 517.065247
#:32.594 [Sync]
.glwindow motion 660 106 512 0
.glwindow posable set Window.main s 0.183909
.glwindow posable set Window.main xyz 9199.095703 14014.007813 526.982178
#:32.625 [Sync]
.glwindow motion 659 104 512 0
.glwindow posable set Window.main s 0.198622
.glwindow posable set Window.main xyz 9312.916016 14303.480469 534.419922
#:32.657 [Sync]
.glwindow motion 658 104 512 0
.glwindow posable set Window.main s 0.214512
.glwindow posable set Window.main xyz 9398.281250 14520.584961 539.998230
#:32.688 [Sync]
.glwindow motion 658 103 512 0
.glwindow posable set Window.main s 0.231672
.glwindow posable set Window.main xyz 9462.304688 14683.413086 544.181946
#:32.719 [Sync]
.glwindow motion 657 102 512 0
.glwindow posable set Window.main s 0.250206
.glwindow posable set Window.main xyz 9510.322266 14805.534180 547.319763
#:32.75 [Sync]
.glwindow posable set Window.main s 0.270223
.glwindow posable set Window.main xyz 9546.335938 14897.125000 549.673096
#:32.782 [Sync]
.glwindow motion 656 102 512 0
.glwindow posable set Window.main s 0.291841
.glwindow posable set Window.main xyz 9573.345703 14965.818359 551.438110
#:32.813 [Sync]
.glwindow posable set Window.main s 0.315188
.glwindow posable set Window.main xyz 9593.603516 15017.337891 552.761841
#:32.844 [Sync]
.glwindow motion 656 101 512 0
.glwindow posable set Window.main s 0.340403
.glwindow posable set Window.main xyz 9608.796875 15055.977539 553.754639
#:32.875 [Sync]
.glwindow posable set Window.main s 0.367635
.glwindow posable set Window.main xyz 9620.191406 15084.957031 554.499268
#:32.907 [Sync]
.glwindow posable set Window.main s 0.397046
.glwindow posable set Window.main xyz 9628.737305 15106.692383 555.057739
#:32.938 [Sync]
.glwindow posable set Window.main s 0.428810
.glwindow posable set Window.main xyz 9635.146484 15122.993164 555.476563
#:32.969 [Sync]
.glwindow posable set Window.main s 0.463115
.glwindow posable set Window.main xyz 9639.954102 15135.218750 555.790710
#:33.016 [Sync]
.glwindow posable set Window.main s 0.500164
.glwindow posable set Window.main xyz 9643.559570 15144.387695 556.026306
#:33.047 [Sync]
.glwindow posable set Window.main s 0.540177
.glwindow posable set Window.main xyz 9646.263672 15151.264648 556.203003
#:33.079 [Sync]
.glwindow posable set Window.main s 0.583391
.glwindow posable set Window.main xyz 9648.291992 15156.422852 556.335510
#:33.125 [Sync]
.glwindow motion 655 101 512 0
.glwindow posable set Window.main s 0.630062
.glwindow posable set Window.main xyz 9649.813477 15160.291016 556.434875
#:33.172 [Sync]
.glwindow posable set Window.main s 0.680467
.glwindow posable set Window.main xyz 9650.954102 15163.192383 556.509399
#:33.219 [Sync]
.glwindow posable set Window.main s 0.734905
.glwindow posable set Window.main xyz 9651.809570 15165.368164 556.565308
#:33.266 [Sync]
.glwindow posable set Window.main s 0.793697
.glwindow posable set Window.main xyz 9652.451172 15167.000000 556.607239
#:33.313 [Sync]
.glwindow motion 654 101 512 0
.glwindow posable set Window.main s 0.857193
.glwindow posable set Window.main xyz 9652.932617 15168.223633 556.638672
#:33.36 [Sync]
.glwindow posable set Window.main s 0.925768
.glwindow posable set Window.main xyz 9653.293945 15169.141602 556.662231
#:33.438 [Sync]
.glwindow posable set Window.main s 0.999830
.glwindow posable set Window.main xyz 9653.564453 15169.830078 556.679932
#:33.516 [Sync]
.glwindow posable set Window.main s 1.079816
.glwindow posable set Window.main xyz 9653.767578 15170.346680 556.693176
#:33.563 [Sync]
.glwindow posable set Window.main s 1.166202
.glwindow posable set Window.main xyz 9653.919922 15170.734375 556.703125
#:33.594 [Sync]
.glwindow posable set Window.main s 1.259498
.glwindow posable set Window.main xyz 9654.034180 15171.024414 556.710571
#:33.657 [Sync]
.glwindow posable set Window.main s 1.360258
.glwindow posable set Window.main xyz 9654.120117 15171.242188 556.716187
#:33.704 [Sync]
.glwindow posable set Window.main s 1.469078
.glwindow posable set Window.main xyz 9654.184570 15171.405273 556.720398
#:33.766 [Sync]
.glwindow posable set Window.main s 1.586605
.glwindow posable set Window.main xyz 9654.232422 15171.527344 556.723572
#:33.829 [Sync]
.glwindow posable set Window.main s 1.713533
.glwindow posable set Window.main xyz 9654.268555 15171.619141 556.725952
#:33.891 [Sync]
.glwindow posable set Window.main s 1.850616
.glwindow posable set Window.main xyz 9654.295898 15171.688477 556.727722
#:33.954 [Sync]
.glwindow posable set Window.main s 1.998665
.glwindow posable set Window.main xyz 9654.316406 15171.740234 556.729065
#:34 [Sync]
.glwindow posable set Window.main s 2.158559
.glwindow posable set Window.main xyz 9654.332031 15171.779297 556.730042
#:34.063 [Sync]
.glwindow posable set Window.main s 2.331243
.glwindow posable set Window.main xyz 9654.343750 15171.808594 556.730774
#:34.125 [Sync]
.glwindow posable set Window.main s 2.517743
.glwindow posable set Window.main xyz 9654.352539 15171.830078 556.731323
#:34.172 [Sync]
.glwindow posable set Window.main s 2.719162
.glwindow posable set Window.main xyz 9654.359375 15171.846680 556.731750
#:34.219 [Sync]
.glwindow posable set Window.main s 2.936696
.glwindow posable set Window.main xyz 9654.364258 15171.859375 556.732056
#:34.266 [Sync]
.glwindow posable set Window.main s 3.171631
.glwindow posable set Window.main xyz 9654.368164 15171.868164 556.732300
#:34.313 [Sync]
.glwindow posable set Window.main s 3.425362
.glwindow posable set Window.main xyz 9654.371094 15171.875000 556.732483
#:34.36 [Sync]
.glwindow posable set Window.main s 3.699391
.glwindow posable set Window.main xyz 9654.373047 15171.879883 556.732605
#:34.391 [Sync]
.glwindow posable set Window.main s 3.995342
.glwindow posable set Window.main xyz 9654.375000 15171.883789 556.732727
#:34.422 [Sync]
.glwindow posable set Window.main s 4.314970
.glwindow posable set Window.main xyz 9654.375977 15171.886719 556.732788
#:34.454 [Sync]
.glwindow posable set Window.main s 4.660168
.glwindow posable set Window.main xyz 9654.376953 15171.888672 556.732849
#:34.485 [Sync]
.glwindow posable set Window.main s 5.032981
.glwindow posable set Window.main xyz 9654.377930 15171.890625 556.732910
#:34.516 [Sync]
.glwindow posable set Window.main s 5.435620
.glwindow posable set Window.main xyz 9654.377930 15171.891602 556.732971
#:34.547 [Sync]
.glwindow posable set Window.main s 5.870470
.glwindow posable set Window.main xyz 9654.377930 15171.892578 556.732971
#:34.594 [Sync]
.glwindow posable set Window.main s 6.340108
.glwindow posable set Window.main xyz 9654.377930 15171.893555 556.732971
#:34.625 [Sync]
.glwindow motion 653 101 512 0
.glwindow posable set Window.main s 6.847317
.glwindow posable set Window.main xyz 9654.377930 15171.894531 556.732971
#:34.657 [Sync]
.glwindow motion 652 101 0 0
.glwindow motion 651 107 0 0
#:34.688 [Sync]
.glwindow motion 650 115 0 0
#:34.719 [Sync]
.glwindow motion 648 126 0 0
.glwindow motion 646 154 0 0
#:34.75 [Sync]
.glwindow motion 640 186 0 0
.glwindow motion 635 220 0 0
#:34.782 [Sync]
.glwindow motion 633 258 0 0
.glwindow motion 628 294 0 0
#:34.813 [Sync]
.glwindow motion 623 328 0 0
.glwindow motion 618 354 0 0
#:34.844 [Sync]
.glwindow motion 612 364 0 0
#:34.875 [Sync]
.glwindow motion 605 369 0 0
.glwindow motion 596 371 0 0
#:34.907 [Sync]
.glwindow motion 587 373 0 0
.glwindow motion 575 374 0 0
#:34.938 [Sync]
.glwindow motion 547 376 0 0
.glwindow motion 535 381 0 0
#:34.969 [Sync]
.glwindow motion 527 383 0 0
.glwindow motion 526 385 0 0
#:35 [Sync]
.glwindow motion 525 385 0 0
#:35.032 [Sync]
.glwindow motion 524 385 0 0
#:35.063 [Sync]
.glwindow motion 523 385 0 0
.glwindow motion 523 384 0 0
#:35.094 [Sync]
.glwindow motion 522 382 0 0
.glwindow motion 520 380 0 0
#:35.125 [Sync]
.glwindow motion 514 378 0 0
.glwindow motion 504 376 0 0
#:35.157 [Sync]
.glwindow motion 494 374 0 0
#:35.188 [Sync]
.glwindow motion 482 373 0 0
.glwindow motion 456 371 0 0
#:35.219 [Sync]
.glwindow motion 444 369 0 0
.glwindow motion 433 368 0 0
#:35.25 [Sync]
.glwindow motion 424 367 0 0
.glwindow motion 419 367 0 0
#:35.282 [Sync]
.glwindow motion 417 366 0 0
#:35.313 [Sync]
.glwindow motion 416 365 0 0
#:35.344 [Sync]
.glwindow motion 416 363 0 0
.glwindow motion 415 362 0 0
#:35.375 [Sync]
.glwindow motion 414 360 0 0
.glwindow motion 409 355 0 0
#:35.407 [Sync]
.glwindow motion 407 350 0 0
.glwindow motion 406 348 0 0
#:35.438 [Sync]
.glwindow motion 405 347 0 0
#:35.469 [Sync]
.glwindow motion 403 345 0 0
.glwindow motion 402 344 0 0
#:35.5 [Sync]
.glwindow motion 402 343 0 0
.glwindow motion 401 342 0 0
#:35.532 [Sync]
#:35.563 [Sync]
#:35.594 [Sync]
#:35.625 [Sync]
#:35.657 [Sync]
.glwindow motion 400 341 0 0
#:35.688 [Sync]
.glwindow motion 399 340 0 0
#:35.719 [Sync]
#:35.75 [Sync]
#:35.782 [Sync]
.glwindow motion 398 339 0 0
#:35.813 [Sync]
#:35.844 [Sync]
#:35.875 [Sync]
#:35.907 [Sync]
.glwindow motion 398 338 0 0
#:35.938 [Sync]
#:35.969 [Sync]
.glwindow mouse 398 338 0 1 0
.glwindow motion 399 339 256 0
.glwindow posable set Window.sub0 xyz 9610.033203 15277.684570 518.927795
#:36.032 [Sync]
#:36.172 [Sync]
.glwindow motion 399 339 256 0
#:36.219 [Sync]
#:36.25 [Sync]
#:36.282 [Sync]
#:36.313 [Sync]
#:36.36 [Sync]
.glwindow motion 401 341 256 0
.glwindow posable set Window.sub0 xyz 9610.441406 15280.697266 516.746033
#:36.391 [Sync]
#:36.422 [Sync]
.glwindow motion 401 342 256 0
.glwindow posable set Window.sub0 xyz 9610.522461 15279.815430 516.699463
#:36.454 [Sync]
#:36.5 [Sync]
#:36.532 [Sync]
.glwindow motion 402 343 256 0
.glwindow posable set Window.sub0 xyz 9615.107422 15242.473633 532.724670
#:36.563 [Sync]
.glwindow motion 403 343 256 0
.glwindow posable set Window.sub0 xyz 9615.470703 15242.436523 532.741211
#:36.594 [Sync]
.glwindow motion 404 344 256 0
.glwindow posable set Window.sub0 xyz 9615.888672 15241.747070 532.653931
#:36.641 [Sync]
.glwindow motion 407 344 256 0
.glwindow posable set Window.sub0 xyz 9618.577148 15226.737305 539.286682
#:36.672 [Sync]
.glwindow motion 410 344 256 0
.glwindow posable set Window.sub0 xyz 9619.541992 15227.339844 539.020447
#:36.704 [Sync]
.glwindow motion 422 344 256 0
.glwindow posable set Window.sub0 xyz 9618.798828 15280.109375 515.701965
#:36.735 [Sync]
.glwindow motion 426 344 256 0
.glwindow posable set Window.sub0 xyz 9620.372070 15280.298828 515.618408
#:36.782 [Sync]
.glwindow motion 429 344 256 0
.glwindow posable set Window.sub0 xyz 9621.553711 15280.440430 515.555664
#:36.813 [Sync]
.glwindow motion 439 344 256 0
.glwindow posable set Window.sub0 xyz 9625.499023 15280.890625 515.356934
#:36.844 [Sync]
.glwindow motion 453 345 256 0
.glwindow posable set Window.sub0 xyz 9635.264648 15211.187500 545.800476
#:36.875 [Sync]
.glwindow motion 461 345 256 0
.glwindow posable set Window.sub0 xyz 9634.117188 15283.512695 513.759888
#:36.922 [Sync]
.glwindow motion 463 345 256 0
.glwindow posable set Window.sub0 xyz 9634.893555 15284.030273 513.530518
#:36.954 [Sync]
.glwindow motion 465 345 256 0
.glwindow posable set Window.sub0 xyz 9635.718750 15283.580078 513.729736
#:36.985 [Sync]
.glwindow motion 472 345 256 0
.glwindow posable set Window.sub0 xyz 9642.968750 15174.396484 562.099487
#:37.016 [Sync]
.glwindow motion 476 344 256 0
.glwindow posable set Window.sub0 xyz 9644.124023 15174.518555 562.361267
#:37.063 [Sync]
.glwindow motion 487 343 256 0
.glwindow posable set Window.sub0 xyz 9647.301758 15174.640625 562.623047
#:37.094 [Sync]
.glwindow motion 502 340 256 0
.glwindow posable set Window.sub0 xyz 9651.377930 15201.215820 551.944824
#:37.125 [Sync]
.glwindow motion 522 336 256 0
.glwindow posable set Window.sub0 xyz 9658.391602 15266.595703 525.016602
#:37.157 [Sync]
.glwindow motion 537 333 256 0
.glwindow posable set Window.sub0 xyz 9664.416016 15278.157227 521.297668
#:37.204 [Sync]
.glwindow motion 548 331 256 0
.glwindow posable set Window.sub0 xyz 9665.915039 15202.718750 554.391357
#:37.235 [Sync]
.glwindow motion 549 331 256 0
.glwindow posable set Window.sub0 xyz 9666.212891 15202.259766 554.587341
#:37.266 [Sync]
.glwindow motion 549 330 256 0
.glwindow posable set Window.sub0 xyz 9666.250000 15203.361328 554.460815
#:37.313 [Sync]
.glwindow motion 550 328 256 0
.glwindow posable set Window.sub0 xyz 9666.625977 15205.118164 554.403198
#:37.344 [Sync]
.glwindow motion 551 324 256 0
.glwindow posable set Window.sub0 xyz 9667.088867 15209.270508 554.039978
#:37.375 [Sync]
.glwindow motion 550 317 256 0
.glwindow posable set Window.sub0 xyz 9670.945313 15316.864258 512.160461
#:37.407 [Sync]
.glwindow motion 547 315 256 0
.glwindow posable set Window.sub0 xyz 9667.815430 15266.276367 533.813049
#:37.454 [Sync]
.glwindow motion 545 314 256 0
.glwindow posable set Window.sub0 xyz 9667.106445 15267.865234 533.572388
#:37.485 [Sync]
.glwindow motion 543 314 256 0
.glwindow posable set Window.sub0 xyz 9666.340820 15267.665039 533.654114
#:37.516 [Sync]
.glwindow motion 542 314 256 0
.glwindow posable set Window.sub0 xyz 9665.958008 15267.565430 533.694885
#:37.547 [Sync]
.glwindow motion 541 314 256 0
.glwindow posable set Window.sub0 xyz 9665.607422 15268.542969 533.295654
#:37.594 [Sync]
.glwindow motion 540 313 256 0
.glwindow posable set Window.sub0 xyz 9665.232422 15268.919922 533.552612
#:37.625 [Sync]
.glwindow motion 539 313 256 0
.glwindow posable set Window.sub0 xyz 9664.810547 15267.422852 534.162537
#:37.657 [Sync]
#:37.688 [Sync]
#:37.735 [Sync]
#:37.766 [Sync]
.glwindow motion 538 313 256 0
.glwindow posable set Window.sub0 xyz 9664.463867 15268.652344 533.661621
#:37.797 [Sync]
#:37.829 [Sync]
#:37.875 [Sync]
#:37.907 [Sync]
#:37.938 [Sync]
#:37.969 [Sync]
#:38.016 [Sync]
#:38.047 [Sync]
#:38.079 [Sync]
.glwindow mouse 538 313 256 1 1
#:38.11 [Sync]
.glwindow motion 537 313 0 0
.glwindow motion 537 314 0 0
#:38.157 [Sync]
#:38.188 [Sync]
#:38.219 [Sync]
.glwindow motion 535 314 0 0
.glwindow motion 525 313 0 0
#:38.25 [Sync]
.glwindow motion 489 312 0 0
.glwindow motion 441 307 0 0
#:38.282 [Sync]
.glwindow motion 383 300 0 0
.glwindow motion 321 294 0 0
#:38.313 [Sync]
.glwindow motion 261 288 0 -1
.sub0.innards.togl motion 190 230 0 1
#:38.344 [Sync]
.sub0.innards.togl motion 150 223 0 0
#:38.375 [Sync]
.sub0.innards.togl motion 120 218 0 0
.sub0.innards.togl motion 110 212 0 0
#:38.407 [Sync]
.sub0.innards.togl motion 103 207 0 0
.sub0.innards.togl motion 97 200 0 0
#:38.438 [Sync]
.sub0.innards.togl motion 92 194 0 0
.sub0.innards.togl motion 90 187 0 0
#:38.469 [Sync]
.sub0.innards.togl motion 89 181 0 0
.sub0.innards.togl motion 89 175 0 0
#:38.5 [Sync]
.sub0.innards.togl motion 89 166 0 0
#:38.532 [Sync]
.sub0.innards.togl motion 89 140 0 0
.sub0.innards.togl motion 90 128 0 0
#:38.563 [Sync]
.sub0.innards.togl motion 91 117 0 0
.sub0.innards.togl motion 92 107 0 0
#:38.594 [Sync]
.sub0.innards.togl motion 94 98 0 0
.sub0.innards.togl motion 96 90 0 0
#:38.625 [Sync]
.sub0.innards.togl motion 102 82 0 0
.sub0.innards.togl motion 107 75 0 0
#:38.657 [Sync]
.sub0.innards.togl motion 107 73 0 0
#:38.688 [Sync]
.sub0.innards.togl motion 108 72 0 0
#:38.719 [Sync]
.sub0.innards.togl motion 108 71 0 0
#:38.75 [Sync]
.sub0.innards.togl motion 109 71 0 0
.sub0.innards.togl motion 111 70 0 0
#:38.782 [Sync]
.sub0.innards.togl motion 117 69 0 0
.sub0.innards.togl motion 123 68 0 0
#:38.813 [Sync]
.sub0.innards.togl motion 129 66 0 0
#:38.844 [Sync]
.sub0.innards.togl motion 135 65 0 0
.sub0.innards.togl motion 141 64 0 0
#:38.875 [Sync]
.sub0.innards.togl motion 146 62 0 0
.sub0.innards.togl motion 151 61 0 0
#:38.907 [Sync]
.sub0.innards.togl motion 152 61 0 0
.sub0.innards.togl motion 152 60 0 0
#:38.938 [Sync]
#:38.969 [Sync]
#:39 [Sync]
.sub0.innards.togl motion 151 59 0 0
#:39.032 [Sync]
.sub0.innards.togl motion 150 59 0 0
#:39.063 [Sync]
.sub0.innards.togl motion 149 59 0 0
#:39.094 [Sync]
#:39.125 [Sync]
.sub0.innards.togl motion 148 59 0 0
#:39.157 [Sync]
#:39.188 [Sync]
.sub0.innards.togl motion 147 59 0 0
#:39.219 [Sync]
.sub0.innards.togl motion 146 59 0 0
.sub0.innards.togl motion 145 59 0 0
#:39.25 [Sync]
#:39.282 [Sync]
.sub0.innards.togl motion 144 58 0 0
#:39.313 [Sync]
#:39.344 [Sync]
.sub0.innards.togl motion 143 58 0 0
#:39.375 [Sync]
#:39.407 [Sync]
.sub0.innards.togl mouse 143 58 0 1 0
#:39.438 [Sync]
#:39.485 [Sync]
.sub0.innards.togl mouse 143 58 256 1 1
#:40.594 [Sync]
.sub0.innards.togl motion 147 67 0 0
#:40.625 [Sync]
#:40.657 [Sync]
.sub0.innards.togl motion 148 67 0 0
#:40.688 [Sync]
#:40.719 [Sync]
.sub0.innards.togl motion 149 67 0 0
#:40.75 [Sync]
.sub0.innards.togl motion 151 66 0 0
#:40.782 [Sync]
.sub0.innards.togl motion 152 65 0 0
#:40.813 [Sync]
#:40.844 [Sync]
#:40.875 [Sync]
#:40.907 [Sync]
#:40.954 [Sync]
.glwindow posable set Window.sub0 xyz 9664.395508 15269.325195 533.892700
#:40.985 [Sync]
.sub0.innards.togl motion 153 64 512 0
.glwindow posable set Window.sub0 xyz 9664.344727 15269.830078 534.066040
#:41.016 [Sync]
.sub0.innards.togl motion 155 57 512 0
.glwindow posable set Window.sub0 s 382.179413
.glwindow posable set Window.sub0 xyz 9664.306641 15270.208984 534.196045
#:41.063 [Sync]
.sub0.innards.togl motion 156 53 512 0
.glwindow posable set Window.sub0 s 412.753784
.glwindow posable set Window.sub0 xyz 9664.278320 15270.493164 534.293518
#:41.094 [Sync]
.sub0.innards.togl motion 156 50 512 0
.glwindow posable set Window.sub0 s 445.774109
.glwindow posable set Window.sub0 xyz 9664.256836 15270.706055 534.366638
#:41.125 [Sync]
.sub0.innards.togl motion 154 47 512 0
.glwindow posable set Window.sub0 s 481.436066
.glwindow posable set Window.sub0 xyz 9664.240234 15270.866211 534.421448
#:41.172 [Sync]
.sub0.innards.togl motion 154 46 512 0
.glwindow posable set Window.sub0 s 519.950989
.glwindow posable set Window.sub0 xyz 9664.228516 15270.986328 534.462585
#:41.204 [Sync]
.glwindow posable set Window.sub0 s 561.547119
.glwindow posable set Window.sub0 xyz 9664.219727 15271.076172 534.493408
#:41.235 [Sync]
.sub0.innards.togl motion 153 45 512 0
.glwindow posable set Window.sub0 s 606.470886
.glwindow posable set Window.sub0 xyz 9664.212891 15271.143555 534.516541
#:41.266 [Sync]
.glwindow posable set Window.sub0 s 654.988586
.glwindow posable set Window.sub0 xyz 9664.208008 15271.194336 534.533875
#:41.313 [Sync]
.glwindow posable set Window.sub0 s 707.387695
.glwindow posable set Window.sub0 xyz 9664.204102 15271.232422 534.546875
#:41.344 [Sync]
.sub0.innards.togl motion 153 44 512 0
.glwindow posable set Window.sub0 s 763.978760
.glwindow posable set Window.sub0 xyz 9664.201172 15271.260742 534.556641
#:41.375 [Sync]
.glwindow posable set Window.sub0 s 825.097107
.glwindow posable set Window.sub0 xyz 9664.199219 15271.282227 534.563965
#:41.407 [Sync]
#:41.438 [Sync]
#:41.469 [Sync]
.sub0.innards.togl motion 153 45 0 0
#:41.5 [Sync]
.sub0.innards.togl motion 153 46 0 0
.sub0.innards.togl motion 153 47 0 0
#:41.532 [Sync]
.sub0.innards.togl motion 154 49 0 0
#:41.563 [Sync]
.sub0.innards.togl motion 155 55 0 0
.sub0.innards.togl motion 157 63 0 0
#:41.594 [Sync]
.sub0.innards.togl motion 159 70 0 0
.sub0.innards.togl motion 161 76 0 0
#:41.625 [Sync]
.sub0.innards.togl motion 162 77 0 0
.sub0.innards.togl motion 163 78 0 0
#:41.657 [Sync]
#:41.688 [Sync]
#:41.719 [Sync]
.sub0.innards.togl motion 164 77 0 0
.sub0.innards.togl motion 164 70 0 0
#:41.75 [Sync]
.sub0.innards.togl motion 164 61 0 0
.sub0.innards.togl motion 164 56 0 0
#:41.782 [Sync]
.sub0.innards.togl motion 163 54 0 0
.sub0.innards.togl motion 163 53 0 0
#:41.813 [Sync]
#:41.844 [Sync]
.sub0.innards.togl motion 163 52 0 0
#:41.875 [Sync]
.sub0.innards.togl motion 163 51 0 0
.sub0.innards.togl motion 162 50 0 0
#:41.907 [Sync]
.sub0.innards.togl motion 162 49 0 0
#:41.938 [Sync]
#:41.969 [Sync]
#:42 [Sync]
#:42.032 [Sync]
.sub0.innards.togl motion 161 49 0 0
#:42.063 [Sync]
#:42.094 [Sync]
#:42.125 [Sync]
.sub0.innards.togl mouse 161 49 0 1 0
#:42.157 [Sync]
#:42.188 [Sync]
.sub0.innards.togl motion 162 49 256 0
#:42.219 [Sync]
.sub0.innards.togl motion 164 49 256 0
.glwindow posable set Window.sub0 h 0.050000
#:42.25 [Sync]
.sub0.innards.togl motion 171 49 256 0
.glwindow posable set Window.sub0 h 0.200000
#:42.282 [Sync]
.sub0.innards.togl motion 178 49 256 0
.glwindow posable set Window.sub0 h 0.700000
#:42.313 [Sync]
.sub0.innards.togl motion 179 49 256 0
.glwindow posable set Window.sub0 h 1.550000
#:42.36 [Sync]
.sub0.innards.togl motion 180 49 256 0
.glwindow posable set Window.sub0 h 2.450000
#:42.391 [Sync]
.sub0.innards.togl motion 181 49 256 0
.glwindow posable set Window.sub0 h 3.400000
#:42.422 [Sync]
.glwindow posable set Window.sub0 h 4.400000
#:42.454 [Sync]
.sub0.innards.togl motion 182 49 256 0
.glwindow posable set Window.sub0 h 5.400000
#:42.5 [Sync]
.sub0.innards.togl motion 184 49 256 0
.glwindow posable set Window.sub0 h 6.450000
#:42.532 [Sync]
.sub0.innards.togl motion 186 49 256 0
.glwindow posable set Window.sub0 h 7.600000
#:42.579 [Sync]
.sub0.innards.togl motion 187 49 256 0
.glwindow posable set Window.sub0 h 8.850000
#:42.61 [Sync]
.sub0.innards.togl motion 188 49 256 0
.glwindow posable set Window.sub0 h 10.150001
#:42.641 [Sync]
.sub0.innards.togl motion 190 49 256 0
.glwindow posable set Window.sub0 h 11.500001
#:42.688 [Sync]
.sub0.innards.togl motion 191 48 256 0
.glwindow posable set Window.sub0 h 12.950001
#:42.719 [Sync]
.sub0.innards.togl motion 192 48 256 0
.glwindow posable set Window.sub0 h 14.450001
#:42.75 [Sync]
.glwindow posable set Window.sub0 h 16.000000
#:42.782 [Sync]
.glwindow posable set Window.sub0 h 17.549999
#:42.829 [Sync]
.sub0.innards.togl motion 193 48 256 0
.glwindow posable set Window.sub0 h 19.099998
#:42.86 [Sync]
.sub0.innards.togl motion 194 48 256 0
.glwindow posable set Window.sub0 h 20.699999
#:42.891 [Sync]
.sub0.innards.togl motion 195 47 256 0
.glwindow posable set Window.sub0 h 22.349998
#:42.922 [Sync]
.sub0.innards.togl motion 196 47 256 0
.glwindow posable set Window.sub0 h 24.049999
#:42.969 [Sync]
.glwindow posable set Window.sub0 h 25.799999
#:43 [Sync]
.sub0.innards.togl motion 197 47 256 0
.glwindow posable set Window.sub0 h 27.549999
#:43.032 [Sync]
.glwindow posable set Window.sub0 h 29.349998
#:43.063 [Sync]
.glwindow posable set Window.sub0 h 31.149998
#:43.11 [Sync]
.sub0.innards.togl motion 198 46 256 0
.glwindow posable set Window.sub0 h 32.949997
#:43.141 [Sync]
.glwindow posable set Window.sub0 h 34.799995
#:43.172 [Sync]
.glwindow posable set Window.sub0 h 36.649994
#:43.219 [Sync]
.glwindow posable set Window.sub0 h 38.499992
#:43.25 [Sync]
.glwindow posable set Window.sub0 h 40.349991
#:43.282 [Sync]
.glwindow posable set Window.sub0 h 42.199989
#:43.313 [Sync]
.glwindow posable set Window.sub0 h 44.049988
#:43.36 [Sync]
.sub0.innards.togl motion 199 46 256 0
.sub0.innards.togl mouse 199 46 256 1 1
.sub0.innards.togl motion 199 47 0 0
#:43.391 [Sync]
.sub0.innards.togl motion 200 48 0 0
#:43.422 [Sync]
.sub0.innards.togl motion 200 49 0 0
.sub0.innards.togl motion 200 54 0 0
#:43.454 [Sync]
.sub0.innards.togl motion 200 61 0 0
.sub0.innards.togl motion 200 70 0 0
#:43.485 [Sync]
.sub0.innards.togl motion 199 78 0 0
.sub0.innards.togl motion 199 83 0 0
#:43.516 [Sync]
.sub0.innards.togl motion 199 88 0 0
.sub0.innards.togl motion 199 94 0 0
#:43.547 [Sync]
.sub0.innards.togl motion 198 100 0 0
.sub0.innards.togl motion 198 105 0 0
#:43.579 [Sync]
.sub0.innards.togl motion 198 107 0 0
#:43.61 [Sync]
.sub0.innards.togl motion 198 108 0 0
#:43.641 [Sync]
#:43.672 [Sync]
.sub0.innards.togl motion 199 108 0 0
.sub0.innards.togl motion 201 108 0 0
#:43.704 [Sync]
.sub0.innards.togl motion 203 108 0 0
.sub0.innards.togl motion 205 108 0 0
#:43.735 [Sync]
.sub0.innards.togl motion 207 110 0 0
#:43.766 [Sync]
.sub0.innards.togl motion 209 116 0 0
.sub0.innards.togl motion 214 124 512 0
.glwindow posable set Window.sub0 s 759.089355
.glwindow posable set Window.sub0 xyz 9664.255859 15271.218750 534.568604
#:43.829 [Sync]
.sub0.innards.togl motion 215 146 512 0
.glwindow posable set Window.sub0 s 698.362244
.glwindow posable set Window.sub0 xyz 9664.298828 15271.171875 534.572083
#:43.86 [Sync]
.sub0.innards.togl motion 215 166 512 0
.glwindow posable set Window.sub0 s 642.493286
.glwindow posable set Window.sub0 xyz 9664.331055 15271.136719 534.574707
#:43.891 [Sync]
.sub0.innards.togl motion 215 180 512 0
.glwindow posable set Window.sub0 s 591.093811
.glwindow posable set Window.sub0 xyz 9664.355469 15271.110352 534.576660
#:43.922 [Sync]
.sub0.innards.togl motion 216 193 512 0
.glwindow posable set Window.sub0 s 543.806335
.glwindow posable set Window.sub0 xyz 9664.373047 15271.090820 534.578125
#:43.969 [Sync]
.sub0.innards.togl motion 216 200 512 0
.glwindow posable set Window.sub0 s 500.301849
.glwindow posable set Window.sub0 xyz 9664.386719 15271.076172 534.579224
#:44 [Sync]
.sub0.innards.togl motion 216 202 512 0
.glwindow posable set Window.sub0 s 460.277710
.glwindow posable set Window.sub0 xyz 9664.396484 15271.064453 534.580078
#:44.032 [Sync]
.sub0.innards.togl motion 216 203 512 0
.glwindow posable set Window.sub0 s 423.455505
.glwindow posable set Window.sub0 xyz 9664.404297 15271.055664 534.580688
#:44.063 [Sync]
.sub0.innards.togl motion 216 204 512 0
.glwindow posable set Window.sub0 s 389.579071
.glwindow posable set Window.sub0 xyz 9664.410156 15271.049805 534.581177
#:44.11 [Sync]
.glwindow posable set Window.sub0 s 358.412750
.glwindow posable set Window.sub0 xyz 9664.414063 15271.044922 534.581543
#:44.141 [Sync]
.glwindow posable set Window.sub0 s 329.739746
.glwindow posable set Window.sub0 xyz 9664.416992 15271.041016 534.581787
#:44.172 [Sync]
.glwindow posable set Window.sub0 s 303.360565
.glwindow posable set Window.sub0 xyz 9664.418945 15271.038086 534.581970
#:44.219 [Sync]
.glwindow posable set Window.sub0 s 279.091736
.glwindow posable set Window.sub0 xyz 9664.420898 15271.036133 534.582092
#:44.25 [Sync]
.glwindow posable set Window.sub0 s 256.764404
.glwindow posable set Window.sub0 xyz 9664.421875 15271.034180 534.582214
#:44.282 [Sync]
.glwindow posable set Window.sub0 s 236.223251
.glwindow posable set Window.sub0 xyz 9664.422852 15271.033203 534.582275
#:44.313 [Sync]
.glwindow posable set Window.sub0 s 217.325394
.glwindow posable set Window.sub0 xyz 9664.423828 15271.032227 534.582336
#:44.36 [Sync]
.glwindow posable set Window.sub0 s 199.939362
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582397
#:44.391 [Sync]
.glwindow posable set Window.sub0 s 183.944214
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.422 [Sync]
.glwindow posable set Window.sub0 s 169.228683
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.454 [Sync]
.glwindow posable set Window.sub0 s 155.690399
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.5 [Sync]
.glwindow posable set Window.sub0 s 143.235168
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.532 [Sync]
.glwindow posable set Window.sub0 s 131.776352
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.563 [Sync]
.glwindow posable set Window.sub0 s 121.234245
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.594 [Sync]
.glwindow posable set Window.sub0 s 111.535507
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.641 [Sync]
.glwindow posable set Window.sub0 s 102.612671
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.672 [Sync]
.glwindow posable set Window.sub0 s 94.403656
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.704 [Sync]
.glwindow posable set Window.sub0 s 86.851364
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.735 [Sync]
.glwindow posable set Window.sub0 s 79.903259
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.782 [Sync]
.glwindow posable set Window.sub0 s 73.511002
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.813 [Sync]
.glwindow posable set Window.sub0 s 67.630119
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.844 [Sync]
.glwindow posable set Window.sub0 s 62.219711
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.875 [Sync]
.sub0.innards.togl motion 217 204 512 0
.glwindow posable set Window.sub0 s 57.242134
.glwindow posable set Window.sub0 xyz 9664.424805 15271.031250 534.582458
#:44.922 [Sync]
.sub0.innards.togl motion 220 203 512 0
.sub0.innards.togl motion 226 203 0 0
.sub0.innards.togl motion 254 203 0 0
#:44.954 [Sync]
.sub0.innards.togl motion 306 209 0 0
.sub0.innards.togl motion 372 219 0 -1
.glwindow motion 393 271 0 1
#:44.985 [Sync]
.glwindow motion 463 282 0 0
#:45.016 [Sync]
.glwindow motion 531 293 0 0
.glwindow motion 583 299 0 0
#:45.047 [Sync]
.glwindow motion 613 301 0 0
.glwindow motion 619 302 0 0
#:45.079 [Sync]
#:45.11 [Sync]
#:45.141 [Sync]
#:45.172 [Sync]
#:45.204 [Sync]
#:45.235 [Sync]
.glwindow motion 618 302 0 0
.glwindow motion 616 303 0 0
#:45.266 [Sync]
.glwindow motion 611 304 0 0
.glwindow motion 603 305 0 0
#:45.297 [Sync]
.glwindow motion 591 305 0 0
#:45.329 [Sync]
.glwindow motion 579 306 0 0
.glwindow motion 572 306 0 0
#:45.36 [Sync]
.glwindow motion 567 306 0 0
.glwindow motion 566 306 0 0
#:45.391 [Sync]
#:45.422 [Sync]
#:45.454 [Sync]
#:45.485 [Sync]
.glwindow motion 565 306 0 0
#:45.516 [Sync]
.glwindow motion 565 307 0 0
.glwindow motion 564 307 0 0
#:45.547 [Sync]
.glwindow motion 563 307 0 0
#:45.579 [Sync]
.glwindow motion 561 308 0 0
.glwindow motion 559 308 0 0
#:45.61 [Sync]
.glwindow motion 557 308 0 0
#:45.641 [Sync]
.glwindow motion 555 308 0 0
.glwindow motion 554 308 0 0
#:45.672 [Sync]
.glwindow motion 553 308 0 0
#:45.704 [Sync]
#:45.735 [Sync]
.glwindow motion 552 308 0 0
.glwindow motion 551 308 0 0
#:45.766 [Sync]
#:45.797 [Sync]
.glwindow motion 550 308 0 0
.glwindow motion 548 308 0 0
#:45.829 [Sync]
.glwindow motion 547 308 0 0
.glwindow motion 546 308 0 0
#:45.86 [Sync]
.glwindow motion 544 308 0 0
#:45.891 [Sync]
.glwindow motion 543 308 0 0
#:45.922 [Sync]
.glwindow motion 542 308 0 0
#:45.954 [Sync]
#:45.985 [Sync]
.glwindow motion 541 308 0 0
#:46.016 [Sync]
.glwindow motion 540 308 0 0
#:46.047 [Sync]
#:46.079 [Sync]
.glwindow motion 539 308 0 0
#:46.11 [Sync]
.glwindow motion 538 308 0 0
#:46.141 [Sync]
.glwindow motion 537 308 0 0
#:46.172 [Sync]
#:46.204 [Sync]
.glwindow motion 536 308 0 0
#:46.235 [Sync]
#:46.266 [Sync]
.glwindow mouse 536 308 0 1 0
.glwindow motion 538 308 256 0
.glwindow posable set Window.sub0 xyz 9664.502930 15270.925781 534.795593
#:46.313 [Sync]
#:46.36 [Sync]
.glwindow motion 518 311 256 0
.glwindow posable set Window.sub0 xyz 9657.205078 15322.541016 512.641846
#:46.407 [Sync]
.glwindow motion 519 311 256 0
.glwindow posable set Window.sub0 xyz 9657.639648 15322.430664 512.686279
#:46.454 [Sync]
.glwindow motion 506 314 256 0
.glwindow posable set Window.sub0 xyz 9652.625000 15207.564453 558.209290
#:46.485 [Sync]
.glwindow motion 503 315 256 0
.glwindow posable set Window.sub0 xyz 9651.666992 15207.704102 557.808044
#:46.516 [Sync]
.glwindow motion 500 317 256 0
.glwindow posable set Window.sub0 xyz 9650.706055 15207.752930 557.098572
#:46.547 [Sync]
.glwindow motion 489 319 256 0
.glwindow posable set Window.sub0 xyz 9644.822266 15310.988281 513.662781
#:46.594 [Sync]
.glwindow motion 476 322 256 0
.glwindow posable set Window.sub0 xyz 9642.511719 15221.750000 549.528503
#:46.625 [Sync]
.glwindow motion 463 325 256 0
.glwindow posable set Window.sub0 xyz 9638.281250 15219.011719 549.593079
#:46.657 [Sync]
.glwindow motion 456 326 256 0
.glwindow posable set Window.sub0 xyz 9631.363281 15299.913086 515.109924
#:46.688 [Sync]
.glwindow motion 453 327 256 0
.glwindow posable set Window.sub0 xyz 9630.210938 15298.213867 515.378601
#:46.735 [Sync]
.glwindow motion 447 329 256 0
.glwindow posable set Window.sub0 xyz 9627.949219 15294.579102 516.026550
#:46.766 [Sync]
.glwindow motion 435 331 256 0
.glwindow posable set Window.sub0 xyz 9623.185547 15292.267578 516.123474
#:46.797 [Sync]
.glwindow motion 422 334 256 0
.glwindow posable set Window.sub0 xyz 9622.897461 15237.115234 538.545593
#:46.829 [Sync]
.glwindow motion 411 336 256 0
.glwindow posable set Window.sub0 xyz 9614.073242 15284.871094 517.104736
#:46.875 [Sync]
.glwindow motion 408 337 256 0
.glwindow posable set Window.sub0 xyz 9612.861328 15284.786133 516.704651
#:46.907 [Sync]
#:46.938 [Sync]
.glwindow motion 407 338 256 0
.glwindow posable set Window.sub0 xyz 9612.544922 15283.825195 516.685486
#:46.969 [Sync]
#:47.016 [Sync]
.glwindow motion 405 338 256 0
.glwindow posable set Window.sub0 xyz 9611.767578 15283.611328 516.778381
#:47.047 [Sync]
.glwindow motion 404 338 256 0
.glwindow posable set Window.sub0 xyz 9611.379883 15283.504883 516.824829
#:47.079 [Sync]
.glwindow motion 404 339 256 0
.glwindow posable set Window.sub0 xyz 9611.454102 15282.662109 516.756287
#:47.11 [Sync]
.glwindow motion 403 339 256 0
.glwindow posable set Window.sub0 xyz 9611.066406 15282.555664 516.802612
#:47.157 [Sync]
#:47.188 [Sync]
#:47.219 [Sync]
.glwindow motion 402 339 256 0
.glwindow posable set Window.sub0 xyz 9610.677734 15282.466797 516.841736
#:47.266 [Sync]
.glwindow motion 402 340 256 0
.glwindow posable set Window.sub0 xyz 9610.753906 15281.623047 516.775024
#:47.297 [Sync]
.glwindow motion 401 340 256 0
.glwindow posable set Window.sub0 xyz 9610.365234 15281.537109 516.812622
#:47.329 [Sync]
.glwindow motion 400 340 256 0
.glwindow posable set Window.sub0 xyz 9609.976563 15281.451172 516.850159
#:47.36 [Sync]
.glwindow motion 399 340 256 0
.glwindow posable set Window.sub0 xyz 9609.587891 15281.363281 516.888794
#:47.407 [Sync]
.glwindow motion 398 340 256 0
.glwindow posable set Window.sub0 xyz 9609.203125 15281.251953 516.937500
#:47.438 [Sync]
#:47.469 [Sync]
#:47.5 [Sync]
#:47.547 [Sync]
#:47.594 [Sync]
#:47.625 [Sync]
#:47.657 [Sync]
#:47.688 [Sync]
#:47.735 [Sync]
#:47.766 [Sync]
#:47.797 [Sync]
#:47.829 [Sync]
#:47.875 [Sync]
.glwindow motion 398 339 256 0
.glwindow posable set Window.sub0 xyz 9609.634766 15277.721680 518.911682
#:47.907 [Sync]
#:47.938 [Sync]
.glwindow motion 398 338 256 0
.glwindow posable set Window.sub0 xyz 9609.654297 15277.719727 519.342224
#:47.969 [Sync]
.glwindow motion 399 338 256 0
.glwindow posable set Window.sub0 xyz 9610.052734 15277.682617 519.358337
#:48.016 [Sync]
#:48.047 [Sync]
#:48.079 [Sync]
#:48.125 [Sync]
#:48.157 [Sync]
#:48.188 [Sync]
#:48.219 [Sync]
#:48.266 [Sync]
#:48.297 [Sync]
#:48.329 [Sync]
.glwindow motion 398 338 256 0
.glwindow posable set Window.sub0 xyz 9609.654297 15277.719727 519.342224
#:48.36 [Sync]
.glwindow motion 397 338 256 0
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:48.407 [Sync]
#:48.438 [Sync]
#:48.469 [Sync]
#:48.5 [Sync]
#:48.547 [Sync]
#:48.579 [Sync]
#:48.61 [Sync]
#:48.641 [Sync]
.glwindow mouse 397 338 256 1 1
#:48.688 [Sync]
#:48.719 [Sync]
#:48.75 [Sync]
#:48.782 [Sync]
#:48.813 [Sync]
#:48.844 [Sync]
.glwindow motion 397 337 0 0
.glwindow motion 396 337 0 0
#:48.875 [Sync]
.glwindow motion 394 335 0 0
.glwindow motion 387 328 0 0
#:48.907 [Sync]
.glwindow motion 378 319 0 0
.glwindow motion 367 307 0 0
#:48.938 [Sync]
.glwindow motion 355 298 0 0
#:48.969 [Sync]
.glwindow motion 349 296 0 0
.glwindow motion 348 295 0 0
#:49 [Sync]
#:49.032 [Sync]
.glwindow motion 347 295 0 0
#:49.063 [Sync]
.glwindow motion 346 294 0 0
#:49.094 [Sync]
.glwindow motion 345 294 0 0
.glwindow motion 343 293 0 0
#:49.125 [Sync]
.glwindow motion 338 291 0 0
.glwindow motion 332 286 0 -1
.sub0.innards.togl motion 311 234 0 1
#:49.157 [Sync]
.sub0.innards.togl motion 304 225 0 0
.sub0.innards.togl motion 295 214 0 0
#:49.188 [Sync]
.sub0.innards.togl motion 287 206 0 0
.sub0.innards.togl motion 282 204 0 0
#:49.219 [Sync]
.sub0.innards.togl motion 277 202 0 0
#:49.25 [Sync]
.sub0.innards.togl motion 275 200 0 0
.sub0.innards.togl motion 270 195 0 0
#:49.282 [Sync]
.sub0.innards.togl motion 264 190 0 0
.sub0.innards.togl motion 262 188 0 0
#:49.313 [Sync]
.sub0.innards.togl motion 261 188 0 0
.sub0.innards.togl motion 260 187 0 0
#:49.344 [Sync]
.sub0.innards.togl motion 258 187 0 0
.sub0.innards.togl motion 252 182 0 0
#:49.375 [Sync]
.sub0.innards.togl motion 244 174 0 0
#:49.407 [Sync]
.sub0.innards.togl motion 235 167 0 0
.sub0.innards.togl motion 228 166 0 0
#:49.438 [Sync]
.sub0.innards.togl motion 227 165 0 0
#:49.469 [Sync]
.sub0.innards.togl motion 226 166 0 0
#:49.5 [Sync]
.sub0.innards.togl motion 224 171 0 0
#:49.532 [Sync]
.sub0.innards.togl motion 219 178 0 0
.sub0.innards.togl motion 213 185 0 0
#:49.563 [Sync]
.sub0.innards.togl motion 207 190 0 0
.sub0.innards.togl motion 202 195 0 0
#:49.594 [Sync]
.sub0.innards.togl motion 200 200 0 0
.sub0.innards.togl motion 199 202 0 0
#:49.625 [Sync]
.sub0.innards.togl motion 198 203 0 0
.sub0.innards.togl motion 197 203 0 0
#:49.657 [Sync]
.sub0.innards.togl motion 197 204 0 0
#:49.688 [Sync]
.sub0.innards.togl motion 196 204 0 0
#:49.719 [Sync]
#:49.75 [Sync]
#:49.782 [Sync]
.sub0.innards.togl motion 196 205 0 0
#:49.844 [Sync]
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:49.891 [Sync]
.sub0.innards.togl motion 194 199 512 0
.sub0.innards.togl motion 192 192 512 0
.glwindow posable set Window.sub0 s 61.821507
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:49.922 [Sync]
.sub0.innards.togl motion 189 175 512 0
.glwindow posable set Window.sub0 s 66.767227
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:49.969 [Sync]
.sub0.innards.togl motion 187 160 512 0
.glwindow posable set Window.sub0 s 72.108604
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50 [Sync]
.sub0.innards.togl motion 185 149 512 0
.glwindow posable set Window.sub0 s 77.877296
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.032 [Sync]
.sub0.innards.togl motion 183 142 512 0
.glwindow posable set Window.sub0 s 84.107483
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.079 [Sync]
.sub0.innards.togl motion 182 141 512 0
.glwindow posable set Window.sub0 s 90.836082
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.11 [Sync]
.sub0.innards.togl motion 182 140 512 0
.glwindow posable set Window.sub0 s 98.102974
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.141 [Sync]
.sub0.innards.togl motion 181 140 512 0
.glwindow posable set Window.sub0 s 105.951218
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.172 [Sync]
.sub0.innards.togl motion 181 139 512 0
.glwindow posable set Window.sub0 s 114.427322
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.219 [Sync]
.glwindow posable set Window.sub0 s 123.581512
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.25 [Sync]
.glwindow posable set Window.sub0 s 133.468033
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.282 [Sync]
.sub0.innards.togl motion 180 139 512 0
.glwindow posable set Window.sub0 s 144.145477
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.313 [Sync]
.glwindow posable set Window.sub0 s 155.677124
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.36 [Sync]
.glwindow posable set Window.sub0 s 168.131302
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.391 [Sync]
.glwindow posable set Window.sub0 s 181.581818
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.422 [Sync]
.glwindow posable set Window.sub0 s 196.108368
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.454 [Sync]
.glwindow posable set Window.sub0 s 211.797043
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.5 [Sync]
.glwindow posable set Window.sub0 s 228.740814
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.532 [Sync]
.glwindow posable set Window.sub0 s 247.040085
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.594 [Sync]
.glwindow posable set Window.sub0 s 266.803314
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.641 [Sync]
.glwindow posable set Window.sub0 s 288.147583
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.672 [Sync]
.glwindow posable set Window.sub0 s 311.199402
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.704 [Sync]
.glwindow posable set Window.sub0 s 336.095367
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.735 [Sync]
.glwindow posable set Window.sub0 s 362.983002
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.782 [Sync]
.sub0.innards.togl motion 180 138 512 0
.glwindow posable set Window.sub0 s 392.021667
.glwindow posable set Window.sub0 xyz 9609.255859 15277.756836 519.326172
#:50.813 [Sync]
#:50.844 [Sync]
#:50.875 [Sync]
#:50.907 [Sync]
#:50.938 [Sync]
.sub0.innards.togl motion 180 137 0 0
.sub0.innards.togl motion 181 136 0 0
#:50.969 [Sync]
.sub0.innards.togl motion 182 134 0 0
.sub0.innards.togl motion 183 129 0 0
#:51 [Sync]
.sub0.innards.togl motion 185 127 0 0
.sub0.innards.togl motion 186 121 0 0
#:51.032 [Sync]
.sub0.innards.togl motion 186 116 0 0
#:51.063 [Sync]
.sub0.innards.togl motion 186 115 0 0
#:51.094 [Sync]
#:51.125 [Sync]
#:51.157 [Sync]
#:51.188 [Sync]
.sub0.innards.togl motion 187 120 0 0
.sub0.innards.togl motion 188 120 0 0
#:51.219 [Sync]
#:51.25 [Sync]
#:51.282 [Sync]
#:51.313 [Sync]
#:51.344 [Sync]
#:51.375 [Sync]
#:51.407 [Sync]
#:51.438 [Sync]
#:51.469 [Sync]
#:51.5 [Sync]
.sub0.innards.togl motion 188 118 0 0
#:51.532 [Sync]
.sub0.innards.togl motion 188 117 0 0
.sub0.innards.togl motion 188 115 0 0
#:51.563 [Sync]
.sub0.innards.togl motion 188 113 0 0
.sub0.innards.togl motion 190 111 0 0
#:51.594 [Sync]
.sub0.innards.togl motion 191 109 0 0
.sub0.innards.togl motion 191 108 0 0
#:51.625 [Sync]
.sub0.innards.togl motion 192 107 0 0
.sub0.innards.togl motion 193 105 0 0
#:51.657 [Sync]
.sub0.innards.togl motion 194 100 0 0
#:51.688 [Sync]
.sub0.innards.togl motion 199 94 0 0
.sub0.innards.togl motion 201 83 0 0
#:51.719 [Sync]
.sub0.innards.togl motion 202 53 0 0
.sub0.innards.togl motion 202 21 0 0
#:51.75 [Sync]
.sub0.innards.togl motion 202 14 0 0
.sub0.innards.togl motion 201 12 0 0
#:51.782 [Sync]
.sub0.innards.togl motion 201 11 0 0
.sub0.innards.togl motion 201 10 0 0
#:51.813 [Sync]
.sub0.innards.togl motion 201 9 0 0
#:51.844 [Sync]
.sub0.innards.togl motion 202 8 0 0
.sub0.innards.togl motion 203 8 0 0
#:51.875 [Sync]
.sub0.innards.togl motion 204 7 0 0
#:51.907 [Sync]
#:51.938 [Sync]
.sub0.innards.togl motion 205 7 0 0
.sub0.innards.togl motion 206 7 0 0
#:51.969 [Sync]
.sub0.innards.togl motion 207 6 0 0
#:52 [Sync]
.sub0.innards.togl motion 208 5 0 0
.sub0.innards.togl motion 209 5 0 0
#:52.032 [Sync]
.sub0.innards.togl motion 210 4 0 0
.sub0.innards.togl motion 211 2 0 0
#:52.063 [Sync]
.sub0.innards.togl motion 213 0 0 0
.sub0.innards.togl motion 218 -2 0 -1
#:52.094 [Sync]
#:52.125 [Sync]
#:52.157 [Sync]
#:52.188 [Sync]
#:52.219 [Sync]
#:52.25 [Sync]
#:52.282 [Sync]
#:52.313 [Sync]
#:52.344 [Sync]
#:52.375 [Sync]
 postSubToglMenu sub0 200 9
#:52.407 [Sync]
#:52.438 [Sync]
#:52.469 [Sync]
#:52.5 [Sync]
#:52.532 [Sync]
#:52.563 [Sync]
#:52.594 [Sync]
#:52.625 [Sync]
#:52.657 [Sync]
#:52.688 [Sync]
#:52.719 [Sync]
#:52.75 [Sync]
.sub0.innards.togl motion 223 6 0 1
#:52.782 [Sync]
#:52.813 [Sync]
#:52.844 [Sync]
#:52.875 [Sync]
#:52.907 [Sync]
#:52.938 [Sync]
#:52.969 [Sync]
#:53 [Sync]
.sub0.innards.togl motion 244 48 0 -1
#:53.032 [Sync]
#:53.063 [Sync]
#:53.094 [Sync]
#:53.125 [Sync]
#:53.157 [Sync]
#:53.188 [Sync]
#:53.219 [Sync]
#:53.25 [Sync]
#:53.282 [Sync]
#:53.313 [Sync]
#:53.344 [Sync]
#:53.375 [Sync]
#:53.407 [Sync]
#:53.438 [Sync]
#:53.469 [Sync]
#:53.5 [Sync]
#:53.532 [Sync]
#:53.563 [Sync]
#:53.594 [Sync]
#:53.625 [Sync]
#:53.657 [Sync]
.sub0.innards.togl motion 241 60 0 1
.sub0.innards.togl minimize
.sub0.innards.togl motion 241 60 0 -1
.glwindow motion 262 112 0 1
#:53.688 [Sync]
#:53.719 [Sync]
#:53.75 [Sync]
#:53.782 [Sync]
#:53.813 [Sync]
#:53.844 [Sync]
#:53.875 [Sync]
.glwindow motion 262 113 0 0
.glwindow motion 264 118 0 0
#:53.907 [Sync]
.glwindow motion 270 125 0 0
.glwindow motion 280 137 0 0
#:53.938 [Sync]
.glwindow motion 292 149 0 0
#:53.969 [Sync]
.glwindow motion 320 179 0 0
.glwindow motion 350 213 0 0
#:54 [Sync]
.glwindow motion 378 253 0 0
.glwindow motion 390 285 0 0
#:54.032 [Sync]
.glwindow motion 400 296 0 0
.glwindow motion 402 301 0 0
#:54.063 [Sync]
.glwindow motion 402 302 0 0
#:54.094 [Sync]
.glwindow motion 402 303 0 0
#:54.125 [Sync]
.glwindow motion 403 304 0 0
.glwindow motion 403 305 0 0
#:54.157 [Sync]
.glwindow motion 403 306 0 0
#:54.188 [Sync]
.glwindow motion 403 308 0 0
#:54.219 [Sync]
.glwindow motion 403 310 0 0
#:54.25 [Sync]
.glwindow motion 403 312 0 0
.glwindow motion 403 314 0 0
#:54.282 [Sync]
.glwindow motion 403 315 0 0
.glwindow motion 403 316 0 0
#:54.313 [Sync]
.glwindow motion 403 317 0 0
#:54.344 [Sync]
.glwindow motion 403 318 0 0
#:54.375 [Sync]
#:54.407 [Sync]
.glwindow motion 402 319 0 0
#:54.438 [Sync]
.glwindow motion 402 320 0 0
.glwindow motion 402 321 0 0
#:54.469 [Sync]
.glwindow motion 402 322 0 0
.glwindow motion 402 324 0 0
#:54.5 [Sync]
.glwindow motion 401 325 0 0
.glwindow motion 401 326 0 0
#:54.532 [Sync]
.glwindow motion 401 327 0 0
#:54.563 [Sync]
.glwindow motion 400 328 0 0
#:54.594 [Sync]
.glwindow motion 399 329 0 0
#:54.625 [Sync]
.glwindow motion 398 329 0 0
#:54.657 [Sync]
.glwindow motion 398 330 0 0
#:54.688 [Sync]
#:54.719 [Sync]
#:54.75 [Sync]
.glwindow motion 398 331 0 0
.glwindow motion 398 332 0 0
#:54.782 [Sync]
.glwindow motion 398 333 0 0
#:54.813 [Sync]
#:54.844 [Sync]
#:54.875 [Sync]
.glwindow motion 398 334 0 0
#:54.907 [Sync]
#:54.938 [Sync]
#:54.969 [Sync]
#:55 [Sync]
#:55.032 [Sync]
#:55.063 [Sync]
.glwindow motion 398 335 0 0
#:55.094 [Sync]
#:55.125 [Sync]
#:55.157 [Sync]
.glwindow mouse 398 335 0 1 0
#:55.188 [Sync]
#:55.219 [Sync]
#:55.25 [Sync]
#:55.282 [Sync]
#:55.313 [Sync]
#:55.344 [Sync]
#:55.375 [Sync]
.glwindow motion 400 334 256 0
.glwindow motion 405 333 256 0
#:55.407 [Sync]
.glwindow motion 410 332 256 0
#:55.438 [Sync]
.glwindow motion 411 332 256 0
.glwindow motion 413 331 256 0
#:55.469 [Sync]
.glwindow motion 415 330 256 0
.glwindow motion 416 330 256 0
#:55.5 [Sync]
.glwindow motion 417 330 256 0
.glwindow motion 419 329 256 0
#:55.532 [Sync]
.glwindow motion 421 329 256 0
.glwindow motion 423 328 256 0
#:55.563 [Sync]
.glwindow motion 425 328 256 0
#:55.594 [Sync]
.glwindow motion 430 327 256 0
.glwindow motion 431 326 256 0
#:55.625 [Sync]
.glwindow motion 433 326 256 0
.glwindow motion 434 326 256 0
#:55.657 [Sync]
.glwindow motion 436 326 256 0
.glwindow motion 437 326 256 0
#:55.688 [Sync]
.glwindow motion 439 326 256 0
.glwindow motion 440 326 256 0
#:55.719 [Sync]
.glwindow motion 442 326 256 0
#:55.75 [Sync]
.glwindow motion 443 326 256 0
.glwindow motion 445 326 256 0
#:55.782 [Sync]
.glwindow motion 446 326 256 0
.glwindow mouse 446 326 256 1 1
.glwindow motion 447 326 0 0
#:55.813 [Sync]
.glwindow motion 448 326 0 0
#:55.844 [Sync]
#:55.875 [Sync]
.glwindow motion 448 327 0 0
#:55.907 [Sync]
.glwindow motion 448 332 0 0
.glwindow motion 448 337 0 0
#:55.938 [Sync]
.glwindow motion 449 344 0 0
.glwindow motion 450 350 0 0
#:55.969 [Sync]
.glwindow motion 451 359 0 0
.glwindow motion 453 371 0 0
#:56 [Sync]
.glwindow motion 454 382 0 0
#:56.032 [Sync]
.glwindow motion 455 390 0 0
.glwindow motion 456 392 0 0
#:56.063 [Sync]
.glwindow motion 457 392 0 0
#:56.094 [Sync]
.glwindow motion 458 392 0 0
.glwindow motion 459 392 0 0
#:56.125 [Sync]
.glwindow motion 460 392 0 0
.glwindow motion 461 392 0 0
#:56.157 [Sync]
#:56.188 [Sync]
.glwindow motion 462 392 0 0
#:56.219 [Sync]
.glwindow motion 463 393 0 0
#:56.25 [Sync]
#:56.282 [Sync]
.glwindow motion 464 393 0 0
#:56.313 [Sync]
#:56.344 [Sync]
#:56.375 [Sync]
#:56.407 [Sync]
#:56.438 [Sync]
#:56.469 [Sync]
#:56.5 [Sync]
#:56.532 [Sync]
#:56.563 [Sync]
#:56.594 [Sync]
#:56.625 [Sync]
#:56.657 [Sync]
#:56.688 [Sync]
#:56.719 [Sync]
#:56.75 [Sync]
#:56.782 [Sync]
#:56.813 [Sync]
#:56.844 [Sync]
#:56.875 [Sync]
.glwindow motion 465 393 0 0
.glwindow motion 456 391 0 0
#:56.907 [Sync]
.glwindow motion 450 390 0 0
#:56.938 [Sync]
#:56.969 [Sync]
.glwindow motion 450 389 0 0
.glwindow motion 450 387 0 0
#:57 [Sync]
.glwindow motion 450 382 0 0
.glwindow motion 449 377 0 0
#:57.032 [Sync]
.glwindow motion 448 376 0 0
.glwindow motion 448 374 0 0
#:57.063 [Sync]
.glwindow motion 446 372 0 0
.glwindow motion 444 365 0 0
#:57.094 [Sync]
.glwindow motion 442 356 0 0
#:57.125 [Sync]
.glwindow motion 440 346 0 0
.glwindow motion 439 340 0 0
#:57.157 [Sync]
.glwindow motion 438 339 0 0
.glwindow motion 437 339 0 0
#:57.188 [Sync]
.glwindow motion 435 339 0 0
.glwindow motion 429 339 0 0
#:57.219 [Sync]
.glwindow motion 423 338 0 0
#:57.25 [Sync]
.glwindow motion 417 337 0 0
.glwindow motion 415 336 0 0
#:57.282 [Sync]
.glwindow motion 410 336 0 0
.glwindow motion 408 336 0 0
#:57.313 [Sync]
.glwindow motion 402 336 0 0
.glwindow motion 397 336 0 0
#:57.344 [Sync]
.glwindow motion 396 336 0 0
.glwindow motion 395 336 0 0
#:57.375 [Sync]
.glwindow motion 394 336 0 0
#:57.407 [Sync]
#:57.438 [Sync]
#:57.469 [Sync]
#:57.5 [Sync]
#:57.532 [Sync]
#:57.563 [Sync]
#:57.594 [Sync]
#:57.625 [Sync]
.glwindow motion 395 336 0 0
#:57.657 [Sync]
.glwindow motion 396 336 0 0
#:57.688 [Sync]
#:57.719 [Sync]
#:57.75 [Sync]
#:57.782 [Sync]
#:57.813 [Sync]
#:57.844 [Sync]
#:57.875 [Sync]
#:57.907 [Sync]
.glwindow motion 397 336 0 0
.glwindow mouse 397 336 0 1 0
#:57.938 [Sync]
#:57.969 [Sync]
.glwindow mouse 397 336 256 1 1
#:58.032 [Sync]
#:58.063 [Sync]
.glwindow mouse 397 336 0 1 0
#:58.094 [Sync]
#:58.125 [Sync]
.glwindow mouse 397 336 256 1 1
#:58.157 [Sync]
#:58.188 [Sync]
#:58.219 [Sync]
#:58.25 [Sync]
#:58.282 [Sync]
#:58.313 [Sync]
#:58.344 [Sync]
#:58.375 [Sync]
#:58.407 [Sync]
#:58.438 [Sync]
#:58.469 [Sync]
#:58.5 [Sync]
#:58.532 [Sync]
#:58.563 [Sync]
#:58.594 [Sync]
#:58.625 [Sync]
.glwindow motion 392 335 0 0
#:58.657 [Sync]
.glwindow motion 391 328 0 0
#:58.688 [Sync]
.glwindow motion 391 317 0 0
.glwindow motion 390 307 0 0
#:58.719 [Sync]
.glwindow motion 388 297 0 0
.glwindow motion 386 285 0 0
#:58.75 [Sync]
.glwindow motion 381 257 0 0
.glwindow motion 375 229 0 0
#:58.782 [Sync]
.glwindow motion 368 221 0 0
.glwindow motion 362 219 0 0
#:58.813 [Sync]
.glwindow motion 361 218 0 0
#:58.844 [Sync]
#:58.875 [Sync]
.glwindow motion 360 218 0 0
#:58.907 [Sync]
.glwindow motion 359 217 0 0
.glwindow motion 357 217 0 0
#:58.938 [Sync]
.glwindow motion 356 216 0 0
#:58.969 [Sync]
#:59 [Sync]
#:59.032 [Sync]
.glwindow motion 355 216 0 0
.glwindow motion 353 215 0 0
#:59.063 [Sync]
.glwindow motion 347 213 0 -1
.sub0.innards.togl motion 326 161 0 1
.sub0.innards.togl motion 318 156 0 0
#:59.094 [Sync]
.sub0.innards.togl motion 309 151 0 0
.sub0.innards.togl motion 301 149 0 0
#:59.125 [Sync]
.sub0.innards.togl motion 296 148 0 0
#:59.157 [Sync]
.sub0.innards.togl motion 290 147 0 0
.sub0.innards.togl motion 283 146 0 0
#:59.188 [Sync]
.sub0.innards.togl motion 275 144 0 0
.sub0.innards.togl motion 268 143 0 0
#:59.219 [Sync]
.sub0.innards.togl motion 261 142 0 0
.sub0.innards.togl motion 254 141 0 0
#:59.25 [Sync]
.sub0.innards.togl motion 249 140 0 0
#:59.282 [Sync]
.sub0.innards.togl motion 243 138 0 0
.sub0.innards.togl motion 235 136 0 0
#:59.313 [Sync]
.sub0.innards.togl motion 226 134 0 0
.sub0.innards.togl motion 220 133 0 0
#:59.344 [Sync]
.sub0.innards.togl motion 219 131 0 0
.sub0.innards.togl motion 218 130 0 0
#:59.375 [Sync]
.sub0.innards.togl motion 217 129 0 0
.sub0.innards.togl motion 216 128 0 0
#:59.407 [Sync]
.sub0.innards.togl motion 216 126 0 0
#:59.438 [Sync]
.sub0.innards.togl motion 215 125 0 0
.sub0.innards.togl motion 215 124 0 0
#:59.469 [Sync]
.sub0.innards.togl motion 215 123 0 0
.sub0.innards.togl motion 214 122 0 0
#:59.5 [Sync]
.sub0.innards.togl motion 214 121 0 0
.sub0.innards.togl motion 213 119 0 0
#:59.532 [Sync]
.sub0.innards.togl motion 212 118 0 0
.sub0.innards.togl motion 211 117 0 0
#:59.563 [Sync]
.sub0.innards.togl motion 211 116 0 0
#:59.594 [Sync]
.sub0.innards.togl motion 210 114 0 0
#:59.625 [Sync]
.sub0.innards.togl motion 209 113 0 0
#:59.657 [Sync]
.sub0.innards.togl mouse 209 113 0 1 0
#:59.688 [Sync]
#:59.719 [Sync]
.sub0.innards.togl motion 208 113 256 0
.sub0.innards.togl mouse 208 113 256 1 1
#:59.797 [Sync]
.sub0.innards.togl motion 208 112 0 0
.sub0.innards.togl motion 207 112 0 0
#:59.829 [Sync]
.sub0.innards.togl motion 206 111 0 0
#:59.86 [Sync]
.sub0.innards.togl motion 205 110 0 0
.sub0.innards.togl motion 203 108 0 0
#:59.891 [Sync]
.sub0.innards.togl motion 201 106 0 0
.sub0.innards.togl motion 194 100 0 0
#:59.922 [Sync]
.sub0.innards.togl motion 187 92 0 0
#:59.954 [Sync]
#:59.985 [Sync]
#:60.016 [Sync]
#:60.047 [Sync]
#:60.079 [Sync]
#:60.11 [Sync]
#:60.141 [Sync]
#:60.172 [Sync]
#:60.204 [Sync]
#:60.235 [Sync]
#:60.266 [Sync]
#:60.297 [Sync]
#:60.329 [Sync]
.sub0.innards.togl motion 177 91 0 0
.sub0.innards.togl motion 151 84 0 0
#:60.36 [Sync]
.sub0.innards.togl motion 143 76 0 0
.sub0.innards.togl motion 142 66 0 0
#:60.391 [Sync]
.sub0.innards.togl motion 143 61 0 0
.sub0.innards.togl motion 144 61 0 0
#:60.422 [Sync]
#:60.454 [Sync]
.sub0.innards.togl motion 145 61 0 0
#:60.485 [Sync]
.sub0.innards.togl motion 146 61 0 0
#:60.516 [Sync]
.sub0.innards.togl motion 146 60 0 0
.sub0.innards.togl motion 146 59 0 0
#:60.547 [Sync]
.sub0.innards.togl motion 147 57 0 0
.sub0.innards.togl motion 146 50 0 0
#:60.579 [Sync]
.sub0.innards.togl motion 144 40 0 0
#:60.61 [Sync]
.sub0.innards.togl motion 142 28 0 0
.sub0.innards.togl motion 141 17 0 0
#:60.641 [Sync]
.sub0.innards.togl motion 140 15 0 0
.sub0.innards.togl motion 139 15 0 0
#:60.672 [Sync]
#:60.704 [Sync]
.sub0.innards.togl motion 140 13 0 0
#:60.735 [Sync]
.sub0.innards.togl motion 140 12 0 0
#:60.766 [Sync]
.sub0.innards.togl motion 141 10 0 0
.sub0.innards.togl motion 141 9 0 0
#:60.797 [Sync]
.sub0.innards.togl motion 141 8 0 0
#:60.829 [Sync]
.sub0.innards.togl motion 141 7 0 0
.sub0.innards.togl motion 141 6 0 0
#:60.86 [Sync]
.sub0.innards.togl motion 141 5 0 0
#:60.891 [Sync]
.sub0.innards.togl motion 140 5 0 0
#:60.922 [Sync]
.sub0.innards.togl motion 139 4 0 0
.sub0.innards.togl motion 138 4 0 0
#:60.954 [Sync]
.sub0.innards.togl motion 138 3 0 0
.sub0.innards.togl motion 136 2 0 0
#:60.985 [Sync]
#:61.016 [Sync]
#:61.047 [Sync]
#:61.079 [Sync]
#:61.11 [Sync]
#:61.141 [Sync]
#:61.172 [Sync]
#:61.204 [Sync]
#:61.235 [Sync]
#:61.266 [Sync]
#:61.297 [Sync]
#:61.329 [Sync]
#:61.36 [Sync]
#:61.391 [Sync]
#:61.422 [Sync]
#:61.454 [Sync]
#:61.485 [Sync]
#:61.516 [Sync]
#:61.547 [Sync]
#:61.579 [Sync]
#:61.61 [Sync]
#:61.641 [Sync]
#:61.672 [Sync]
#:61.704 [Sync]
#:61.735 [Sync]
#:61.766 [Sync]
#:61.797 [Sync]
#:61.829 [Sync]
.sub0.innards.togl keyboard 136 2 0  0 65307
#:61.86 [Sync]
#:61.891 [Sync]
.sub0.innards.togl keyboard 136 2 0 {} 1 65307
#:61.922 [Sync]
#:61.954 [Sync]
#:61.985 [Sync]
#:62.11 [Sync]
#:62.141 [Sync]
.sub0.innards.togl motion 135 1 0 0
#:62.172 [Sync]
.sub0.innards.togl motion 134 -1 0 -1
#:62.204 [Sync]
#:62.235 [Sync]
.glwindow motion 154 28 0 1
.glwindow motion 154 20 0 0
#:62.266 [Sync]
.glwindow motion 155 14 0 0
.glwindow motion 156 9 0 0
#:62.297 [Sync]
.glwindow motion 156 3 0 0
#:62.329 [Sync]
.glwindow motion 156 1 0 0
#:62.36 [Sync]
.glwindow motion 156 0 0 0
#:62.391 [Sync]
.glwindow motion 156 -1 0 -1
#:62.422 [Sync]
#:62.454 [Sync]
#:62.485 [Sync]
#:62.516 [Sync]
#:62.547 [Sync]
#:62.579 [Sync]
#:62.61 [Sync]
#:62.641 [Sync]
#:62.672 [Sync]
#:62.704 [Sync]
#:62.735 [Sync]
#:62.766 [Sync]
#:62.797 [Sync]
#:62.829 [Sync]
#:62.86 [Sync]
#:62.891 [Sync]
#:62.922 [Sync]
#:62.954 [Sync]
#:62.985 [Sync]
#:63.016 [Sync]
#:63.047 [Sync]
#:63.079 [Sync]
#:63.11 [Sync]
#:63.141 [Sync]
#:63.172 [Sync]
#:63.204 [Sync]
#:63.235 [Sync]
#:63.266 [Sync]
#:63.297 [Sync]
#:63.329 [Sync]
#:63.36 [Sync]
#:63.391 [Sync]
#:63.422 [Sync]
#:63.454 [Sync]
#:63.485 [Sync]
#:63.516 [Sync]
#:63.547 [Sync]
#:63.579 [Sync]
#:63.61 [Sync]
#:63.641 [Sync]
#:63.672 [Sync]
#:63.704 [Sync]
#:63.735 [Sync]
#:63.766 [Sync]
#:63.797 [Sync]
#:63.829 [Sync]
#:63.86 [Sync]
#:63.891 [Sync]
#:63.922 [Sync]
.sub0.innards.togl keyboard 140 -61 0 q 0 113
