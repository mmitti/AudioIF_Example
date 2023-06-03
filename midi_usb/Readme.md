
# 注意事項
## ベンダーID
このサンプルでは検証用ベンダーIDを設定しています。
実際に評価される際は所有しているベンダーIDに書き換えてください。
(descriptor.cppのUSB_DC_ID_VENDORを変更してください。)

## デバイスドライバー
このサンプルはUSB標準使用に準拠して作成されています。
そのためUSB標準ドライバーが自動的に適用されるはずですが、環境により自動でドライバーが適用されない場合があります。
その場合は手動で標準ドライバーを適用してください。

Windowsの場合、標準USBオーディオデバイスを選択してください。

また、実装が怪しいため全てのOS、環境で動作するものではありません。