const addon = require('bindings')('main');
const port = 8080;
const url = 'http://localhost:' + port + '/';
const express = require('express');
const app = express();

app.use(express.static('public')).use(express.json());

app.listen(port, () => {
    console.log(`Server is running on port ${port}`);
	var start = (process.platform == 'darwin'? 'open': process.platform == 'win32'? 'start': 'xdg-open');
	require('child_process').exec(start + ' ' + url);
});

app.post('/user', function(request, response){
	var user_priv_level = addon.get_user_priv(request.body.user)
	const user_priv_str = ["Гость", "Пользователь", "Администратор"][user_priv_level]

	if (user_priv_level < 0) {
		// No such user
		response.status(404);
		response.send();
	} else {
		response.status(200);
		response.send({priv:user_priv_str}); 
	}
 });
