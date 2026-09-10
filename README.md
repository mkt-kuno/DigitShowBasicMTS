# DigitShowBasicMTS - 中空ねじり三軸試験版 (OpenSource Edition, for Modbus RTU)

![Github License](https://img.shields.io/github/license/mkt-kuno/DigitShowBasicMTS)  [![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com) 

<img width="1021" height="722" alt="Screenshot 2026-08-22 205927" src="https://github.com/user-attachments/assets/ce4530ed-263e-4517-8498-f80bac3726bd" />

## 簡単な説明
東京大学の地盤研で使用されている、中空ねじり三軸試験機制御ソフトウェアのオープンソース版です。  
[DigitShowBasic](https://github.com/mkt-kuno/DigitShowBasic)を中空ねじり三軸試験用に拡張した版で、中空円筒供試体に軸荷重とねじりトルクを独立して負荷します。  
Modbus RTU接続のAD/DA装置で動作させることを前提としています。  
[DigitShowBasicM](https://github.com/mkt-kuno/DigitShowBasicM)のModbus RTU実装パターンをベースに、中空ねじり三軸試験向けのチャンネル割り当てへ移植しています。  
このリポジトリのライセンスは **GPLv3** となっているため、注意してください。  

## 動作環境
- Windows 11  
x64のみ
- Visual Studio 2022
Community版でOK, MFCライブラリ必須  
- Modbus RTU対応シリアル接続 (COMポート)
  - 38400 bps / 8N1
  - AI: Function Code 0x04, 16ch int16 input registers (HX711 ch0-7 / ADS1115 ch8-15)
  - AO: Function Code 0x10, 8ch uint16 holding registers (GP8403, 0-10000 mV)
- CPU: x64 Intel/AMD問わず  
[Passmark性能(マルチスレッド)](https://www.cpubenchmark.net/multithread/) 最低5000 推奨8000以上
- RAM: 最低4GB 推奨8GB以上  
他に動かすアプリケーション次第。MS Officeは重い。
- GPU: 依存なし、iGPU/dGPU/APU いずれも可
- 液晶: 最低XGA 推奨FHD以上  
縦長画面だと表示が見切れると思います。
- 記憶媒体: 最低HDD 推奨SSD  
容量はビルドPCと動作PCが同じなら最低128GB 推奨256GB


## 注意点
- 本ソフトウェアの動作について、一切の保証を行いません。
- 本ソフトウェアの使用により発生したいかなる損害についても、一切の責任を負いません。
- 本ソフトウェアを使用する場合は、自己責任で行ってください。
- 本ソフトウェアの動作または初期設定についてのサポートは行いません。
- 本ソフトウェアの改変、再配布はGPLv3の条件に従って行ってください。

## ライセンスについて
一部の大学・企業・研究所では秘伝のタレ状態のDigitShowBasicをお持ちだと思います。  
当時(2010年頃)配布されたDigitShowBasicのソースコードは何もライセンスが決められていませんでした。  
そのため、お持ちの古いDigitShowBasicは高確率でライセンスフリーのハズです。  
ですが、このリポジトリはGPLv3です。このコードを安易に参考・参照・引用した場合GPLv3に感染するので、  
GPLv3とは何か知ったうえで、覚悟して使い始めてください。  
ざっくりといえば、改変部分がある場合、ソースコードを公開する必要があります。

## バグ報告やプルリクエストについて
デバッガ、コーダ、メンテナ、などなどが複数人、現れた場合のみ、管理・サポートを行おうと思います。  
「どう使うの？」「ボードが認識しない」「設定方法を教えてほしい」などの初歩的な質問は避けてください。無視します。  
「うちのコードとかなり違う」「そもそも動作しないし落ちる」などの場合は、  
AI協業でリファクタリングする前の[legacy版](https://github.com/mkt-kuno/DigitShowBasicMTS/tree/legacy)で試してみて下さい。  
「初期設定や困った部分を文章化したので載せてほしい」「AIOボードの初期化を自動にしたコードをマージしてほしい」など、  
貢献する意思のある、オープンソースの理念に沿った要求は大歓迎します。  
「根幹設計から新しいの作りたい」というやる気とコーディング能力のある方は、  
[ぜひこちら](https://github.com/mkt-kuno/DigitShowSystem/issues/3)、もしくは東大地盤研までご連絡を。

## リポジトリの運用方針について
- 基本的に新機能の追加は行いません。プルリクがあればコードレビューはします。
- AIによるバイブコーディングを禁止しませんが、推奨もしません。
- Pull Requestを送る場合は、簡単で良いので動作確認を行い、変更点を記載してください。
- バグなどのIssueを送る場合は、必ず考えつく限り詳細な、問題を再現するのに必要な情報を提供してください。
- main/developブランチが荒れない事が第一なので、差分が多くなければfeature/****などに独自のforkコードを置いてOKです。
- Git Worktreeに必ずしも従う必要はありません。
- commitは何となく変更点が分かればいいです。コメントの書き方も自由で良いです。日本語でもOKです。
- 無茶な要求が続くようであれば公開をやめ、コードを放棄します。libxml2のように。

## 技術的特記事項

### Modbus RTU I/O 仕様
- 通信設定: **38400 bps / 8N1**
- COMポート名でオープンします（例: `COM3`）
- AIは **16ch の int16 入力レジスタ**を Function Code **0x04** で一括読取
  - ch0-7: HX711
  - ch8-15: ADS1115
- AOは **8ch の uint16 保持レジスタ**を Function Code **0x10** で一括書込
  - ch0-7: GP8403
  - 値は **mV単位 0-10000** に丸め・クランプ

### AO チャンネル割り当て
- ch0: Axial Motor ON/OFF
- ch1: Axial Motor UP/DOWN
- ch2: Axial Motor Speed
- ch3: EP Cell Pressure
- ch4: EP Axis Pressure
- ch5: Torsional Motor ON/OFF
- ch6: Torsional Motor CW/CCW
- ch7: Torsional Motor Speed

### AI チャンネル割り当て
- ch0: V.Load
- ch1: V.Disp
- ch2: LDT1
- ch3: LDT2
- ch4: T.Load
- ch5: CG1
- ch6: CG2
- ch7: CG3
- ch8: HCDPT
- ch9: LCDPT
- ch10: T.Disp
- ch11-ch15: 予備

### 補足
- 旧CONTEC/CAIO向けのFIFOバッファ計測は廃止し、Timerごとの**ポーリング読取**へ変更しました。
- 現状の実装では **EP Axis Pressure(ch4)** は **EP Cell Pressure(ch3)** の指令値を既定でミラーします。
- 旧POT1/POT2の2系統回転計測は、新しい暫定AIマップの **T.Disp(ch10)** 1系統へ集約しています。