# smart-smartmeter

ESP-32 code to read out a smartmeter using an IR-Photodiode. Decode the values and upload together with some useless device data into a mongoDB (atlas free tier).

project is based on: https://jimb-cc.medium.com/esp-and-mongodb-sitting-in-a-tree-7d043fb1a4d (https://github.com/jimb-cc/houseplant/)

# known issues
once in a while the query from the smartmeter decodes into a wrong value. Checksum or some poor logic could be used to solve.