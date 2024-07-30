# qmk_spacemouse

* qmk版本為0.25.8，沒有驗證其他版本能否使用

* 新增3dconnexion spacemouse的report descriptor

* 將qmk_firmware內資料夾複製到qmk安裝目錄底下，
* 在config.h加入一行 #define SPACEMOUSE
* 在info.json內確定為3dconnexion的id
  "pid": "0xC631",
  "vid": "0x256F"
* 編譯 make ergoknife:via

* ergoknife2是將spacemouse的操作功能映射到觸控板上，其他鍵盤要使用可以參考裡面的寫法
* host_3Dmouse1_send(&report3dm1);
* host_3Dmouse2_send(&report3dm2);

