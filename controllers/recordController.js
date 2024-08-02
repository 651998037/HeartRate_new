const moment = require("moment");
const controller = {};
const { name } = require("ejs");
const { validationResult } = require("express-validator");
const { InfluxDB, Point } = require("@influxdata/influxdb-client");
const token =
  "862S8K00YV8zxnYdVwgZ5kv41FIv1zgIF4s_bst_YpYUinnZQhiwWMuKFKv2RBWaBp1XOEvAFvt7QE9RRmPywQ==";
const url = "https://us-east-1-1.aws.cloud2.influxdata.com";
const client = new InfluxDB({ url, token });
let org = `74554ba6d34f8f12`;
let bucket = `Projectfinished`;

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

controller.Dist = async (req, res) => {
  let HeartRatevalue = null;
  let spo2value = null;

  try {
    const queryClient = client.getQueryApi(org);

    const fluxQuery = `
      from(bucket: "Projectfinished")
        |> range(start: -10m)
        |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
        |> filter(fn: (r) => r._measurement == "HeartRate" and (r["_field"] == "heartRate" or r["_field"] == "spo2"))
        |> last()
    `;

    await new Promise((resolve, reject) => {
      queryClient.queryRows(fluxQuery, {
        next: (row, tableMeta) => {
          const tableObject = tableMeta.toObject(row);
          console.log(tableObject._value);
          if (tableObject._field === "heartRate") {
            HeartRatevalue = tableObject._value;
          } else if (tableObject._field === "spo2") {
            spo2value = tableObject._value;
          }
        },
        error: (error) => {
          console.error("\nError", error);
          reject(error);
        },
        complete: () => {
          resolve();
        },
      });
    });

    if (HeartRatevalue !== null) {
      const point = new Point("HeartRate").intField("HeartRate", HeartRatevalue);

      await new Promise((resolve) => setTimeout(resolve, 1000));
      writeClient.writePoint(point);

      await new Promise((resolve) => setTimeout(resolve, 4000));
      writeClient.flush();
    }

    res.render("Dist", { data: { HeartRatevalue, spo2value } });
  } catch (error) {
    console.error("Error", error);
    res.render("Dist", { data: { HeartRatevalue, spo2value } });
  }
};


controller.graph = async (req, res) => {
  let HeartRatevalue = [];
  let spo2value = [];

  try {
    const queryClient = client.getQueryApi(org);
    const fluxQuery = `
      from(bucket: "Projectfinished")
      |> range(start: -1y)
      |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
      |> filter(fn: (r) => r._measurement == "HeartRate" and (r["_field"] == "heartRate" or  r["_field"] == "spo2"))
    `;

    await new Promise((resolve, reject) => {
      queryClient.queryRows(fluxQuery, {
        next: (row, tableMeta) => {
          const tableObject = tableMeta.toObject(row);
          tableObject.time = new Date(tableObject._time).toLocaleString();

          if (tableObject._field === "heartRate") {
            HeartRatevalue.push(tableObject);
          } else if (tableObject._field === "spo2") {
            spo2value.push(tableObject);
          }
        },
        error: (error) => {
          console.error("\nError", error);
          reject(error);
        },
        complete: () => {
          resolve();
        },
      });
    });

    res.render("graph", { HeartRatevalue, spo2value });
  } catch (error) {
    console.error("Error", error);
    res.render("graph", { HeartRatevalue, spo2value });
  }
};




// controller.graph2 = async (req, res) => {
//   let IRData = []; // Declare IRData array
//   let BPMData = []; // Declare BPMData array
//   let AvgBPMData = []; // Declare AvgBPMData array

//   try {
//       const queryClient = client.getQueryApi(org);

//       const fluxQuery = `
//           from(bucket: "THESABAN")
//           |> range(start: -1y)
//           |> filter(fn: (r) => r["device"] == "8:f9:e0:6c:55:93")
//           |> filter(fn: (r) => r._measurement == "HeartRate" and (r["_field"] == "IR" or r["_field"] == "BPM" or r["_field"] == "AvgBPM"))
//       `;

//       await new Promise((resolve, reject) => {
//           queryClient.queryRows(fluxQuery, {
//               next: (row, tableMeta) => {
//                   const tableObject = tableMeta.toObject(row);
//                   tableObject.time = new Date(tableObject._time).toLocaleString(); // Add date and time information

//                   if (tableObject._field === "IR") {
//                       IRData.push(tableObject);
//                       // console.log("IRvalue:", tableObject._value);
//                   } else if (tableObject._field === "BPM") {
//                       BPMData.push(tableObject);
//                       // console.log("BPMvalue:", tableObject._value);
//                   } else if (tableObject._field === "AvgBPM") {
//                       AvgBPMData.push(tableObject);
//                       // console.log("AvgBPMvalue:", tableObject._value);
//                   }

                 

//               },
//               error: (error) => {
//                   console.error("\nError", error);
//                   reject(error);
//               },
//               complete: () => {
//                   // Render the view with the complete data
//                   resolve();
//               },
//           });
//       });

//       res.render("graph2", { IRData, BPMData, AvgBPMData });
//   } catch (error) {
//       console.error("Error", error);
//       res.render("graph2", { IRData, BPMData, AvgBPMData });

//   }
// };














module.exports = controller;
