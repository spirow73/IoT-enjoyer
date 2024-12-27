import { Router } from 'express';
import { getData, addData } from './controllers/dataController.js';

const router = Router();

router.get('/data', getData);
router.post('/data', addData);

export default router;
