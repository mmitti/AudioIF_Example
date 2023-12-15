# 自作USBオーディオインターフェース と オーディオ基板向けサンプル (Zynq用)

このリポジトリは USBとARMで作る自作USBオーディオインターフェース と 青のペンギン堂製オーディオ基板向けのサンプルコードを格納したリポジトリです。

# ライセンスと利用条件

ソースコードおよびそのほかのリソースはファイル/フォルダーに明示されている場合を除き MITベースライセンス(商用利用および有償なアマチュア製品などへの組み込み時のみMITライセンスに加え追加条項が適用されます)で頒布されています。

本サンプルは手元の環境で動作確認を実施しておりますが、動作を保証するものではありません。
また、本サンプルに関するサポートは原則ありませんのであしからず(issueを立てていただくと余力があれば対応します)

# 動作確認環境
* Windows 11
* Punq-Z1(Zynq-7020)
* Vivado 2023.2

# ファイル構成

* ip: IP(各機能をまとめた回路コード一式)格納場所
    * midi
        * bus2midi: MidiBusからMIDI信号を生成する回路
        * midi: AXI経由でMidiBusの入出力を行なう回路
        * midi2bus: MIDI信号からMidiBusへ変換する回路
        * midiclock: MIDI信号用クロック生成回路
        * hdl: IP共通コード置き場
        * if: IP間のインターフェース定義
* cpp: CPU向け共通ライブラリ格納場所
    * usbps: 標準のZynq向けUSBドライバー(usbps)のバグ修正とC++インターフェース化したもの
* midi_pl_standalone: FPGA(PL)単体で動作するMIDI IPのサンプル回路 入力されたMIDI信号を改変して出力を行なうサンプルです
* midi_pl_ps_standalone: FPGA(PL)とCPU(PS)を併用して動作するMIDI IPのサンプル回路とプログラム 入力されたMIDI信号を一定期間遅延させて出力を行なうサンプルです
* usb_midi: FPGA(PL)とCPU(PS)とUSBコントローラー(PS)を併用して動作するUSB MIDIデバイスのサンプルです。USB経由で受信したMIDI信号をPL経由で出力し、外部から入力されたMIDI信号をUSB経由でホストPCへ転送します。

# 発行物とサンプルの対応表

## 自作USBオーディオインターフェース #1 USB-MIDIデバイス編

以下のサンプルは本誌で解説した内容に関連したものとなります。

* midi_pl_standalone
* midi_pl_ps_standalone
* usb_midi

本誌で解説した各内容と対応する回路は以下の通りです。

|章見出し|IP|
|:---:|:---:|
|MIDI回路(FPGA) (1) AXI-MIDI回路|ip/midi/midi|
|MIDI回路(FPGA) (2) MIDI Clock Gen回路|ip/midi/midiclock|
|MIDI回路(FPGA) (3) Bus2MIDI回路|ip/midi/bus2midi|
|MIDI回路(FPGA) (4) MIDI2Bus回路|ip/midi/midi2bus|

本誌で解説したUSB MIDIファームウェアは usb_midi/cpp以下のソースコードと対応します。

なお、本誌での擬似コードはUSBコントローラー固有の実装は省いたものとなりますが、本サンプルのソースコードではZynqのUSBコントローラー固有の制御処理を含みます。
また、本誌での解説の段階ではUSB 1.1相当での実装となりましたが、このサンプルではUSB2.0相当での実装となります。一部ディスクリプターやパケット処理がUSB2.0向けに修正されておりますのでご注意ください。


## 自作USBオーディオインターフェース #2 USB-AUDIO ファームウェア編
TBD

## mi Audio I/O Board (Main) v1.0

### MIDI入出力機能
以下のサンプルはMIDI入出力機能を利用して動作させることが可能です。

* midi_pl_standalone
* midi_pl_ps_standalone
* usb_midi

### フットスイッチ・フットペダル入力機能
TBD

### S/PDIF入出力機能
TBD

### I2S ADC/DAC 音声入出力機能
TBD
