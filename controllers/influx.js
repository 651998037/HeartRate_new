const moment = require("moment");
const controller = {};
const { name } = require("ejs");
const { validationResult } = require("express-validator");
const {InfluxDB, Point} = require('@influxdata/influxdb-client')
const token = "862S8K00YV8zxnYdVwgZ5kv41FIv1zgIF4s_bst_YpYUinnZQhiwWMuKFKv2RBWaBp1XOEvAFvt7QE9RRmPywQ=="
const url = 'https://us-east-1-1.aws.cloud2.influxdata.com'
const client = new InfluxDB({url, token})
let org = `74554ba6d34f8f12`
let bucket = `Projectfinished`

let writeClient = client.getWriteApi(org, bucket, "ns");


// let point = new Point('dashboard37')
// // .tag('device','abc')
// .intField('IR', 99);
// void setTimeout(() => {
// writeClient.writePoint(point)
// }, 1000) // separate points by 1 second
// void setTimeout(() => {
// writeClient.flush()
// }, 5000)



controller.influx = async (req, res) => {
  let heartRateData = [];
  let spo2Data = [];
  let queryClient = client.getQueryApi(org);

  // Query สำหรับ HeartRate
  let heartRateQuery = `from(bucket: "Projectfinished")
    |> range(start: -1h)
    |> filter(fn: (r) => r._measurement == "HeartRate")
    |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
    |> filter(fn: (r) => r._field == "heartRate")
    |> aggregateWindow(every: 20s, fn: last, createEmpty: true)
    |> yield(name: "last")`;

  // Query สำหรับ SpO2
  let spo2Query = `from(bucket: "Projectfinished")
    |> range(start: -1h)
    |> filter(fn: (r) => r._measurement == "HeartRate")
    |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
    |> filter(fn: (r) => r._field == "spo2")
    |> aggregateWindow(every: 20s, fn: last, createEmpty: true)
    |> yield(name: "last")`;

  // Query สำหรับ HeartRate
  queryClient.queryRows(heartRateQuery, {
    next: (row, tableMeta) => {
      const tableObject = tableMeta.toObject(row);
      heartRateData.push(tableObject._value);
    },
    error: (error) => {
      console.error("\nError", error);
    },
    complete: () => {
      // Query สำหรับ SpO2
      queryClient.queryRows(spo2Query, {
        next: (row, tableMeta) => {
          const tableObject = tableMeta.toObject(row);
          spo2Data.push(tableObject._value);
        },
        error: (error) => {
          console.error("\nError", error);
        },
        complete: () => {
          // console.log({ heartRateData, spo2Data });
          res.render("chart", { heartRateData, spo2Data });
        },
      });
    },
  });
};

controller.influx_1d_31 = async (req, res) => {
  try {
      let data = {
          daily: {
              heartRate: [],
              spo2: []
          },
          weekly: {
              heartRate: [],
              spo2: []
          },
          monthly: {
              heartRate: [],
              spo2: []
          }
      };

      let queryClient = client.getQueryApi(org);

      const queries = {
          heartRate: {
              daily: `from(bucket: "Projectfinished")
                  |> range(start: -24h)
                  |> filter(fn: (r) => r._measurement == "HeartRate")
                  |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
                  |> filter(fn: (r) => r._field == "heartRate")
                  |> aggregateWindow(every: 1h, fn: mean, createEmpty: true)
                  |> yield(name: "mean")`,

              weekly: `from(bucket: "Projectfinished")
                  |> range(start: -7d)
                  |> filter(fn: (r) => r._measurement == "HeartRate")
                  |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
                  |> filter(fn: (r) => r._field == "heartRate")
                  |> aggregateWindow(every: 6h, fn: mean, createEmpty: true)
                  |> yield(name: "mean")`,

              monthly: `from(bucket: "Projectfinished")
                  |> range(start: -30d)
                  |> filter(fn: (r) => r._measurement == "HeartRate")
                  |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
                  |> filter(fn: (r) => r._field == "heartRate")
                  |> aggregateWindow(every: 24h, fn: mean, createEmpty: true)
                  |> yield(name: "mean")`
          },
          spo2: {
              daily: `from(bucket: "Projectfinished")
                  |> range(start: -24h)
                  |> filter(fn: (r) => r._measurement == "HeartRate")
                  |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
                  |> filter(fn: (r) => r._field == "spo2")
                  |> aggregateWindow(every: 1h, fn: mean, createEmpty: true)
                  |> yield(name: "mean")`,

              weekly: `from(bucket: "Projectfinished")
                  |> range(start: -7d)
                  |> filter(fn: (r) => r._measurement == "HeartRate")
                  |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
                  |> filter(fn: (r) => r._field == "spo2")
                  |> aggregateWindow(every: 6h, fn: mean, createEmpty: true)
                  |> yield(name: "mean")`,

              monthly: `from(bucket: "Projectfinished")
                  |> range(start: -30d)
                  |> filter(fn: (r) => r._measurement == "HeartRate")
                  |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
                  |> filter(fn: (r) => r._field == "spo2")
                  |> aggregateWindow(every: 24h, fn: mean, createEmpty: true)
                  |> yield(name: "mean")`
          }
      };

      const fetchData = (query, key, field) => {
          return new Promise((resolve, reject) => {
              queryClient.queryRows(query, {
                  next: (row, tableMeta) => {
                      const tableObject = tableMeta.toObject(row);
                      data[key][field].push(tableObject._value);
                  },
                  error: (error) => {
                      console.error('\nError', error);
                      reject(error);
                  },
                  complete: () => {
                      resolve();
                  },
              });
          });
      };

      await Promise.all([
          fetchData(queries.heartRate.daily, 'daily', 'heartRate'),
          fetchData(queries.heartRate.weekly, 'weekly', 'heartRate'),
          fetchData(queries.heartRate.monthly, 'monthly', 'heartRate'),
          fetchData(queries.spo2.daily, 'daily', 'spo2'),
          fetchData(queries.spo2.weekly, 'weekly', 'spo2'),
          fetchData(queries.spo2.monthly, 'monthly', 'spo2')
      ]);

      res.render('chart_1-31d', { data });
  } catch (error) {
      console.error(error);
      res.status(500).send(error);
  }
};



module.exports = controller;
