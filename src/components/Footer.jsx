import React from 'react';
import { Box, Container, Typography } from '@mui/material';

function Footer() {
  return (
    <Box sx={{ bgcolor: '#0a1929', color: 'white', py: 4, mt: 8 }}>
      <Container maxWidth="lg" sx={{ textAlign: 'center' }}>
        <Typography variant="body2">
          © {new Date().getFullYear()} HERMES. All rights reserved.
        </Typography>
      </Container>
    </Box>
  );
}

export default Footer;