const express = require('express');
const router = express.Router();
const influx = require('../controllers/influx');
// const validator = require('../controllers/validator');


router.get('/influx',influx.influx);
router.get('/influx_1d_31',influx.influx_1d_31);

// router.get('/graph2',recordController.graph2);



module.exports = router;