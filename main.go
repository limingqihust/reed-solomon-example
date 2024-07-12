package main

import (
    "bytes"
    "fmt"
    "io/ioutil"
    "time"

    "github.com/klauspost/reedsolomon"
)

func main() {
    // 输入文件名和输出文件名
    inputFileName := "Input10000"
    encodedFileName := "Input_encode"
    decodedFileName := "Input_decode"

    // 读取输入文件内容
    data, err := ioutil.ReadFile(inputFileName)
    if err != nil {
        fmt.Println("Error reading input file:", err)
        return
    }

    // 创建一个Reed-Solomon编码器
    enc, err := reedsolomon.New(20, 2) // 4 data shards, 2 parity shards
    if err != nil {
        fmt.Println("Error creating encoder:", err)
        return
    }

    // 将数据分片
    shards, err := enc.Split(data)
    fmt.Printf("%d %d\n", len(shards), len(shards[0]))
    if err != nil {
        fmt.Println("Error splitting data:", err)
        return
    }

    startTime := time.Now()
    // 编码数据
    err = enc.Encode(shards)
    if err != nil {
        fmt.Println("Error encoding data:", err)
        return
    }

    duration := time.Since(startTime)
    fmt.Printf("duration: %v\n", duration)

    // 将编码后的分片写入文件
    var encodedBuffer bytes.Buffer
    err = enc.Join(&encodedBuffer, shards, len(data))
    if err != nil {
        fmt.Println("Error joining encoded data:", err)
        return
    }
    err = ioutil.WriteFile(encodedFileName, encodedBuffer.Bytes(), 0644)
    if err != nil {
        fmt.Println("Error writing encoded file:", err)
        return
    }

    fmt.Println("Data encoded and written to", encodedFileName)

    // 读取编码文件内容
    encodedData, err := ioutil.ReadFile(encodedFileName)
    if err != nil {
        fmt.Println("Error reading encoded file:", err)
        return
    }

    // 将编码数据分片
    shards, err = enc.Split(encodedData)
    if err != nil {
        fmt.Println("Error splitting encoded data:", err)
        return
    }

    // 恢复数据
    err = enc.Reconstruct(shards)
    if err != nil {
        fmt.Println("Error reconstructing data:", err)
        return
    }

    // 将恢复的分片合并为原始数据
    var decodedBuffer bytes.Buffer
    err = enc.Join(&decodedBuffer, shards, len(data))
    if err != nil {
        fmt.Println("Error joining decoded data:", err)
        return
    }

    // 将解码后的数据写入文件
    err = ioutil.WriteFile(decodedFileName, decodedBuffer.Bytes(), 0644)
    if err != nil {
        fmt.Println("Error writing decoded file:", err)
        return
    }

    fmt.Println("Data decoded and written to", decodedFileName)
}
