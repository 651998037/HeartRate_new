const express=require('express');
const body=require('body-parser'); 
const cookie=require('cookie-parser');
const session=require('express-session');
const mysql=require('mysql');
const connection=require('express-myconnection');
const app=express();
const path = require('path');
const upload =require("express-fileupload");
const uuidv4 = require('uuid');


app.use(upload());


app.use(express.static('public'));

app.set('view engine','ejs');

app.get('/',function(req,res){
    res.render('upload');

});



app.post('/upload',function(req,res){
    if(req.files){
        var file = req.files.filename;
        //var filename = file.name
        if(!Array.isArray(file)){
            var filename = uuidv4.v4()+"."+file.name.split(".")[1];
        file.mv("./public/"+filename,function(err){
            if(err){console.log(err)}
        })
        
    }else{

        for(var i=0; i < file.length; i++){
        var filename = uuidv4.v4()+"."+file[i].name.split(".")[1];
        file[i].mv("./public/"+filename,function(err){
            if(err){console.log(err)}
        })
    }
}
}
    res.redirect('/');
});


const express=require('express');
const body=require('body-parser'); 
const cookie=require('cookie-parser');
const session=require('express-session');
const mysql=require('mysql');
const connection=require('express-myconnection');
const app=express();
const path = require('path');
const upload =require("express-fileupload");
const uuidv4 = require('uuid');


app.use(upload());


app.use(express.static('public'));

app.set('view engine','ejs');

app.get('/',function(req,res){
    res.render('upload');

});



app.post('/upload',function(req,res){
    if(req.files){
        var file = req.files.filename;
        //var filename = file.name
        if(!Array.isArray(file)){
            var filename = uuidv4.v4()+"."+file.name.split(".")[1];
        file.mv("./public/"+filename,function(err){
            if(err){console.log(err)}
        })
        
    }else{

        for(var i=0; i < file.length; i++){
        var filename = uuidv4.v4()+"."+file[i].name.split(".")[1];
        file[i].mv("./public/"+filename,function(err){
            if(err){console.log(err)}
        })
    }
}
}
    res.redirect('/');
});





app.set('view engine','ejs');
app.use(express.static('public'));
app.use(body.urlencoded({extended: true})); 
app.use(cookie());
app.use(session({
    secret:'12',
    resave:true,
    saveUninitialized: true
}));

app.use(connection(mysql,{
    host:'localhost',
    user:'iot',
    password:'Dakar@05052546!',
    port:'3306',
    database:'osiot'
},'single'));   

// app.get("/g", function (req, res) {
//     req.getConnection((err, conn) => {
//       conn.query("SELECT * FROM HeartRate ", (err, dbdata) => {
//         if (err) {
//           res.json(err);
//         }
//         const dataList = JSON.parse(JSON.stringify(dbdata));
//         let ids = [];
//         dataList.forEach((element) => {
//           ids.push(element.data);
//         });
//         console.log(ids);
//         res.render("chart", { data: ids });
//       });
//     });
//   });


let downloadCounter = 1;
app.get('/iot/HeartRate', async (request, response) => {
//console.log(request);
const mac = (request.header("x-esp8266-sta-mac"));
const version = (request.header("x-esp8266-version"))?parseFloat(request.header("x-esp8266-version")):0;
const smac = mac.toLowerCase();
//console.log(smac);
const datapoint = "HeartRate";
const value = await db.query('SELECT * FROM device JOIN hardware ON device.hardwareId=hardware.hwid JOIN software ON software.swid=device.softwareid WHERE mac=$1 AND datapoint=$2'>
if(err){res.json(err);}
});
let dbversion = 0;
let swid=0;
if(value.rows.length>0){
dbversion = value.rows[0].version;
swid = value.rows[0].swid;
did = value.rows[0].did;
hwid = value.rows[0].hwid;
}
// console.log(did+" ("+hwid+") NOW :"+version+" DB :"+dbversion+"("+swid+")");
if (version>0 && dbversion>0 && !(version==dbversion)){
response.status(200).download(path.join(__dirname,'iot/HeartRate/'+dbversion+'.bin'), dbversion+'.bin', (err)=>{
if (err) {
console.error("Problem on download firmware: ", err)
}else{
const value = db.query('INSERT INTO softwarehistory(deviceid,hardwareid,softwareid,installdate) VALUES ($1,$2,$3,NOW())',[did,hwid,swid]);
downloadCounter++;
}
});
console.log('Your file has been downloaded '+downloadCounter+' times!')
}else{
// console.log('not update!');
response.status(304).send('No update needed!')
}
});







const recordRoute = require('./routes/recordRoute'); 
app.use('/',recordRoute);
const influx = require('./routes/influx'); 
app.use('/',influx);








app.listen('8048');


app.set('view engine','ejs');
app.use(express.static('public'));
app.use(body.urlencoded({extended: true})); 
app.use(cookie());
app.use(session({
    secret:'12',
    resave:true,
    saveUninitialized: true
}));

app.use(connection(mysql,{
    host:'localhost',
    user:'iot',
    password:'Dakar@05052546!',
    port:'3306',
    database:'osiot'
},'single'));   

// app.get("/g", function (req, res) {
//     req.getConnection((err, conn) => {
//       conn.query("SELECT * FROM HeartRate ", (err, dbdata) => {
//         if (err) {
//           res.json(err);
//         }
//         const dataList = JSON.parse(JSON.stringify(dbdata));
//         let ids = [];
//         dataList.forEach((element) => {
//           ids.push(element.data);
//         });
//         console.log(ids);
//         res.render("chart", { data: ids });
//       });
//     });
//   });










const recordRoute = require('./routes/recordRoute'); 
app.use('/',recordRoute);
const influx = require('./routes/influx'); 
app.use('/',influx);








app.listen('8048');