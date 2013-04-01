public Response getResponse(long timeout) throws InterruptedException {
	synchronized (lock) {
		if (response != null) {
			return response;
		}

		lock.wait(timeout);
	}

	reader.removeInputHandler(this);
	return response;
}

		
		